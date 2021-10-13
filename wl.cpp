#include <chrono>
#include <iostream>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <boost/algorithm/string/predicate.hpp>

#include <Graph.h>
//#include "src/AuxiliaryMethods.h"
#include "src/ColorRefinementAmenability.h"


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
            int index = atoi(args[1].c_str());
            vector<string> edge_args = get_args(args[0]);
            assert(edge_args.size() == 2);
            string src = edge_args[0];
            string dst = edge_args[1];
            pair<string, string> edge(src, dst);
            if( map_edge_labels.find(edge) == map_edge_labels.end() )
                map_edge_labels.emplace(edge, vector<int>{});
            map_edge_labels[edge].push_back(index);
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

    // make and return graph
    Labels labels(num_nodes, 0);
    EdgeList edge_list(num_edges);
    for( map<pair<string, string>, int>::const_iterator it = map_edge.begin(); it != map_edge.end(); ++it ) {
        Node src = map_node[it->first.first];
        Node dst = map_node[it->first.second];
        edge_list[it->second] = { src, dst };
    }
    return GraphLibrary::Graph(num_nodes, edge_list, labels);
}

int main(int argc, const char **argv) {
    if( argc != 2 ) {
        cout << "Usage: " << argv[0] << " <filename>" << endl;
        return 0;
    }

    string filename = argv[1];
    cout << "Reading file '" << filename << "' ..." << endl;


    GraphLibrary::GraphDatabase graph_db;
    if( boost::algorithm::ends_with(filename, ".lp") ) {
        // reading a graph description from clingo file (ext .lp)
        ifstream ifs(filename, ifstream::in);
        if( ifs.is_open() ) {
            GraphLibrary::Graph g = read_lp_graph(ifs);
            cout << "graph: #nodes=" << g.get_num_nodes() << ", #edges=" << g.get_num_edges() << endl;
            graph_db.emplace_back(move(g));
            ifs.close();
        } else {
            cout << "Error: opening file '" << filename << "'" << endl;
            return -1;
        }
    }

    auto start = chrono::high_resolution_clock::now();
    for( size_t i = 0; i < graph_db.size(); ++i ) {
        const GraphLibrary::Graph &g = graph_db[i];
        ColorRefinementAmenability::ColorRefinementAmenability cra(g);

        // Manages colors of stable coloring.
        unordered_set<Label> node_colors;
        // Maps color to nodes.
        unordered_multimap<Label, Node> colors_to_nodes;
        // Maps node to color.
        unordered_map<Node, Label> node_to_color;

        // Compute stable coloring
        Labeling labeling = cra.compute_stable_coloring(node_colors, colors_to_nodes, node_to_color);
    }

#if 0
    GraphDatabase graph_data_base = AuxiliaryMethods::read_graph_txt_file("NCI1");
    cout << "Graph data base loaded." << endl;

    double num_is_amenable = 0;
    for (const auto g: graph_data_base) {
        ColorRefinementAmenability::ColorRefinementAmenability cra(g);
        if (cra.check_amenability()) {
            num_is_amenable += 1;
        }
    }

    cout << num_is_amenable / graph_data_base.size() * 100.0 << endl;
    cout << "Running time [s]: " << chrono::duration<double>(end - start).count() / graph_data_base.size() << endl;
#endif
    auto end = chrono::high_resolution_clock::now();
    double elapsed = chrono::duration<double>(end - start).count();
    cout << "WL coloring calculated in " << elapsed << " seconds" << endl;

    return 0;
}
