#include <chrono>
#include <iostream>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>

#include "Graph.h"
#include "ColorRefinement.h"


using namespace std;

string join(const vector<string> &args) {
    string result;
    for( size_t i = 0; i < args.size(); ++i ) {
        result += args[i];
        if( 1 + i < args.size() ) result += ",";
    }
    return result;
}

vector<string> get_args(const string &atom) {
    vector<string> args;
    size_t pos = atom.find_first_of("("), n = 0, rbegin = 0;
    for( size_t i = pos, len = atom.size(); i < len; ++i ) {
        if( (n == 0) && (atom.at(i) == '(') ) {
            ++n;
            rbegin = i + 1;
        } else if( atom.at(i) == '(' ) {
            if( n == 1 ) rbegin = i;
            ++n;
        } else if( (n == 1) && ((atom.at(i) == ',') || atom.at(i) == ')') ) {
            int rlen = i - rbegin;
            string arg = atom.substr(rbegin, rlen);
            //cout << "arg=|" << arg << "|" << endl;
            args.emplace_back(move(arg));
            if( atom.at(i) == ',' )
                rbegin = i + 1;
            else
                --n;
        } else if( atom.at(i) == ')' ) {
            assert(n > 0);
            --n;
        }
    }
    assert(n == 0);
    return args;
}

GraphLibrary::Graph read_lp_graph(ifstream &ifs) {
    using boost::algorithm::starts_with;
    using boost::algorithm::ends_with;

    map<string, Node> map_node;
    map<int, string> map_label;
    map<pair<string, string>, int> map_edge;
    map<pair<string, string>, vector<int> > map_edge_labels;

    string ifs_line;
    while( getline(ifs, ifs_line) ) {
        if( starts_with(ifs_line, "node(") ) {
            vector<string> args = get_args(ifs_line);
            assert(args.size() == 1);
            string node = join(args);
            assert(map_node.find(node) == map_node.end());
            map_node.emplace(node, map_node.size());
        } else if( starts_with(ifs_line, "labelname(") ) {
            vector<string> args = get_args(ifs_line);
            assert(args.size() == 2);
            int index = atoi(args[0].c_str());
            const string &label = args[1];
            assert(map_label.find(index) == map_label.end());
            map_label.emplace(index, label);
        } else if( starts_with(ifs_line, "edge(") ) {
            vector<string> args = get_args(ifs_line);
            assert(args.size() == 1);
            vector<string> edge_args = get_args(args[0]);
            assert(edge_args.size() == 2);
            string src = edge_args[0];
            string dst = edge_args[1];
            pair<string, string> edge(src, dst);
            assert(map_edge.find(edge) == map_edge.end());
            map_edge.emplace(edge, map_edge.size());
        } else if( starts_with(ifs_line, "tlabel(") ) {
            vector<string> args = get_args(ifs_line);
            assert(args.size() == 2);
            int label = atoi(args[1].c_str());
            vector<string> edge_args = get_args(args[0]);
            assert(edge_args.size() == 2);
            string src = edge_args[0];
            string dst = edge_args[1];
            pair<string, string> edge(src, dst);
            if( map_edge_labels.find(edge) == map_edge_labels.end() )
                map_edge_labels.emplace(edge, vector<int>{});
            map_edge_labels[edge].push_back(label);
        }
    }

    int num_nodes = map_node.size();
    int num_edges = map_edge.size();
    int num_labels = map_label.size();
    cout << "graph: #nodes=" << num_nodes << ", #edges=" << num_edges << ", #edge-labels=" << num_labels << endl;

    // check consistency
    for( map<pair<string, string>, int>::const_iterator it = map_edge.begin(); it != map_edge.end(); ++it ) {
        assert(map_node.find(it->first.first) != map_node.end());
        assert(map_node.find(it->first.second) != map_node.end());
    }
    for( map<pair<string, string>, vector<int> >::const_iterator it = map_edge_labels.begin(); it != map_edge_labels.end(); ++it ) {
        assert(map_node.find(it->first.first) != map_node.end());
        assert(map_node.find(it->first.second) != map_node.end());
        for( size_t i = 0; i < it->second.size(); ++i )
            assert(map_label.find(it->second[i]) != map_label.end());
    }

    // normalize edge labels
    map<int, int> remap_label;
    for( map<int, string>::const_iterator it = map_label.begin(); it != map_label.end(); ++it )
        remap_label.emplace(it->first, remap_label.size());

    // construct and return graph
    Labels node_labels(num_nodes, 1);
    Labels edge_labels;
    Nodes edges_src, edges_dst;
    for( map<pair<string, string>, int>::const_iterator it = map_edge.begin(); it != map_edge.end(); ++it ) {
        edges_src.push_back(map_node.at(it->first.first));
        edges_dst.push_back(map_node.at(it->first.second));
        const vector<int> &labels = map_edge_labels.at(it->first);
        assert(labels.size() == 1);
        edge_labels.push_back(remap_label.at(labels.front()));
    }
    return GraphLibrary::Graph(num_nodes, edges_src, edges_dst, edge_labels, node_labels, true);
}

int main(int argc, const char **argv) {
    if( argc != 2 ) {
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return 0;
    }

    //cout << "Max label number is " << numeric_limits<ulong>::max() << endl;
    string filename = argv[1];
    cout << "Reading file '" << filename << "' ..." << endl;

    GraphLibrary::GraphDatabase graph_db;
    if( boost::algorithm::ends_with(filename, ".lp") ) {
        // reading a graph description from clingo file (ext .lp)
        ifstream ifs(filename, ifstream::in);
        if( ifs.is_open() ) {
            GraphLibrary::Graph g = read_lp_graph(ifs);
            graph_db.emplace_back(move(g));
            ifs.close();
        } else {
            cout << "Error: opening file '" << filename << "'" << endl;
            return -1;
        }
    } else {
        cout << "Error: unrecognized file extension" << endl;
        return -1;
    }

    for( size_t i = 0; i < graph_db.size(); ++i ) {
        const GraphLibrary::Graph &g = graph_db[i];
        ColorRefinement::ColorRefinement cra(g);

        // Manages colors of stable coloring.
        unordered_set<Label> node_colors;
        // Maps color to nodes.
        unordered_multimap<Label, Node> colors_to_nodes;
        // Maps node to color.
        unordered_map<Node, Label> node_to_color;

        // Compute stable coloring.
        auto start = chrono::high_resolution_clock::now();
        cra.compute_stable_coloring(node_colors,
                                    colors_to_nodes,
                                    node_to_color,
                                    g.get_node_labels(),
                                    g.get_set_edge_labels().size(),
                                    g.get_edge_labels());
        auto end = chrono::high_resolution_clock::now();
        double elapsed = chrono::duration<double>(end - start).count();
        cout << "WL: #colors=" << node_colors.size() << ", elapsed-time=" << elapsed << endl;

        // Print summary of coloring.
        uint total = 0;
        for( Label label : node_colors ) {
            uint n = colors_to_nodes.count(label);
            total += n;
            cout << n << " node(s) with color " << label << endl;
        }
        cout << "total " << total << " node(s)" << endl;
    }

    return 0;
}
