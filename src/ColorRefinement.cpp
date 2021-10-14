/**********************************************************************
 * Copyright (C) 2017 Christopher Morris <christopher.morris@udo.edu>
 *
 * This file is part of amenability_test. An implementation of the algorithm described in:
 *
 * @InProceedings{Arvind+2015,
 *   author = "Arvind, V. and K{\"o}bler, Johannes and Rattan, Gaurav and Verbitsky, Oleg",
 *   title = "On the Power of Color Refinement",
 *   booktitle = "20th International Symposium on Fundamentals of Computation Theory",
 *   year = "2015",
 *   pages = "339--350"
 * }
 *
 * Amenability_test can not be copied or distributed without the express
 * permission of Christopher Morris.
 *
 * Modified by Blai Bonet (c) 2021
 *********************************************************************/

#include <algorithm>
#include <queue>
#include "ColorRefinement.h"

using namespace std;

namespace ColorRefinement {
    void ColorRefinement::compute_stable_coloring(unordered_set<Label> &node_colors,
                                                  unordered_multimap<Label, Node> &colors_to_nodes,
                                                  unordered_map<Node, Label> &node_to_color,
                                                  const vector<Label> &node_labels,
                                                  uint num_edge_labels,
                                                  const vector<Label> &edge_labels) const {
        // Assumption: edge labels are in { 0, ..., num_edge_labels - 1}.
        for( Label label : edge_labels )
            assert(label < num_edge_labels);

        // Need node labels { 1, ... } since color 0 is marker
        bool add_one_to_node_labels = false;
        for( Label label : node_labels ) {
            if( label == 0 ) {
                add_one_to_node_labels = true;
                break;
            }
        }

        Node num_nodes = m_graph.get_num_nodes();
        assert(node_labels.size() == num_nodes);
        Labels coloring(num_nodes, 0);
        Labels new_coloring(node_labels);
        uint num_old_colors = -1;
        uint num_new_colors = 1; // just to make it different

        if( add_one_to_node_labels ) {
            for( size_t i = 0; i < num_nodes; ++i )
                ++new_coloring[i];
        }

        unordered_set<Label> new_colors;
        while( num_new_colors != num_old_colors ) {
            // Update coloring.
            coloring = new_coloring;
            num_old_colors = num_new_colors;
            new_colors.clear();

            // Iterate over all nodes.
            for( Node v = 0; v < num_nodes; ++v ) {
                // It is assumed that edge colors are in { 0, ..., num_edge_labels-1 }.
                const Edges &outbound_edges = m_graph.get_outbound_edges(v);
                const Edges &inbound_edges = m_graph.get_inbound_edges(v);
                vector<Labels> colors(2 * num_edge_labels);

                // Process edges by color while getting colors of neighbors.
                for( Edge e : outbound_edges ) {
                    assert(m_graph.get_src(e) == v);
                    Node n = m_graph.get_dst(e);
                    Label edge_color = edge_labels.at(e);
                    colors[edge_color].push_back(coloring[n]);
                }
                for( Edge e : inbound_edges ) {
                    assert(m_graph.get_dst(e) == v);
                    Node n = m_graph.get_src(e);
                    Label edge_color = edge_labels.at(e);
                    colors[num_edge_labels + edge_color].push_back(coloring[n]);
                }

                // Insert marker (color 0) in each group and sort them.
                for( uint i = 0; i < num_edge_labels; ++i ) {
                    colors[i].push_back(0);
                    colors[num_edge_labels + i].push_back(0);
                    sort(colors[i].begin(), colors[i].end());
                    sort(colors[num_edge_labels + i].begin(), colors[num_edge_labels + i].end());
                }

                // Compute new label using composition to bijectively map two integers to on integer.
                // Initial color is color of vertex v as this must be taken into account as well.
                Label new_color = coloring[v];
                for( uint i = 0; i < num_edge_labels; ++i ) {
                    for( Label c : colors[i] )
                        new_color = pairing(new_color, c);
                    for( Label c : colors[num_edge_labels + i] )
                        new_color = pairing(new_color, c);
                }
                new_colors.insert(new_color);
                new_coloring[v] = new_color;
            }
            num_new_colors = new_colors.size();
        }

        node_colors = new_colors;
        for( Node i = 0; i < num_nodes; ++i ) {
            colors_to_nodes.insert({{new_coloring[i], i}});
            node_to_color.insert({{i, new_coloring[i]}});
        }
    }

    void ColorRefinement::compute_stable_coloring(unordered_set<Label> &node_colors,
                                                  unordered_multimap<Label, Node> &colors_to_nodes,
                                                  unordered_map<Node, Label> &node_to_color) const {
        return compute_stable_coloring(node_colors, colors_to_nodes, node_to_color, vector<Label>(m_graph.get_num_nodes(), 1), 1, vector<Label>(m_graph.get_num_edges(), 0));
    }
}

