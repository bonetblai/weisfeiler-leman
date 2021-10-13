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

#ifndef GRAPH_H
#define GRAPH_H

#include <algorithm>
#include <unordered_set>
#include <vector>

using namespace std;

using uint = unsigned int;
using ulong = unsigned long;
using Node = uint;
using Nodes = vector<Node>;
using Label = ulong;
using Labels = vector<Label>;
using Edge = pair<Node, Node>;
using EdgeList = vector<Edge>;

namespace std {
    namespace {
        // Code from boost: Reciprocal of the golden ratio helps spread entropy and handles duplicates.
        // See Mike Seymour in magic-numbers-in-boosthash-combine: http://stackoverflow.com/questions/4948780 .
        template<class T>
        inline void hash_combine(std::size_t &seed, T const &v) {
            seed ^= hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        // Recursive template code derived from Matthieu M.
        template<class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
        struct HashValueImpl {
            static void apply(size_t &seed, Tuple const &tuple) {
                HashValueImpl<Tuple, Index - 1>::apply(seed, tuple);
                hash_combine(seed, get<Index>(tuple));
            }
        };

        template<class Tuple>
        struct HashValueImpl<Tuple, 0> {
            static void apply(size_t &seed, Tuple const &tuple) {
                hash_combine(seed, get<0>(tuple));
            }
        };
    }

    // Hash tuples.
    template<typename ... TT>
    struct hash<std::tuple<TT...>> {
        size_t
        operator()(std::tuple<TT...> const &tt) const {
            size_t seed = 0;
            HashValueImpl<std::tuple<TT...> >::apply(seed, tt);
            return seed;
        }
    };

    // Hash pairs.
    template<typename S, typename T>
    struct hash<pair<S, T>> {
        inline size_t operator()(const pair<S, T> &v) const {
            size_t seed = 0;
            hash_combine(seed, v.first);
            hash_combine(seed, v.second);
            return seed;
        }
    };
}

namespace GraphLibrary {
    class Graph {
      public:
        Graph(bool directed = false)
          : m_num_nodes(0),
            m_num_edges(0),
            m_node_labels(),
            m_directed(directed) {
        }
        Graph(const uint num_nodes,
              const EdgeList &edgeList,
              const Labels node_labels,
              bool directed = false)
          : m_adjacency_lists_outbound(),
            m_adjacency_lists_inbound(),
            m_num_nodes(num_nodes),
            m_num_edges(0),
            m_node_labels(node_labels),
            m_directed(directed) {
            m_adjacency_lists_outbound.resize(num_nodes);
            m_adjacency_lists_inbound.resize(num_nodes);
            for( auto const &e : edgeList )
                add_edge(e.first, e.second);
        }

        // Add a single node to the graph.
        size_t add_node() {
            m_adjacency_lists_outbound.push_back(vector<Node>{ });
            m_adjacency_lists_inbound.push_back(vector<Node>{ });
            return m_num_nodes++;
        }

        // Add a single edge to the graph.
        void add_edge(const Node v, const Node w) {
            m_adjacency_lists_outbound[v].push_back(w);
            m_adjacency_lists_inbound[w].push_back(v);
            ++m_num_edges;
            if( !m_directed ) {
                m_adjacency_lists_outbound[w].push_back(v);
                m_adjacency_lists_inbound[v].push_back(w);
                ++m_num_edges;
            }
        }

        // Get degree of node "v".
        size_t get_degree(const Node v) const {
            return m_directed ? get_out_degree(v) + get_in_degree(v) : get_out_degree(v);
        }
        size_t get_in_degree(const Node v) const {
            return m_adjacency_lists_outbound[v].size();
        }
        size_t get_out_degree(const Node v) const {
            return m_adjacency_lists_inbound[v].size();
        }

        // Get neighbors of node "v".
        const Nodes& get_neighbors(const Node v) const {
            return m_adjacency_lists_outbound[v];
        }

        // Get number of nodes in graph.
        size_t get_num_nodes() const {
            return m_num_nodes;
        }

        // Get number of edges in graph.
        size_t get_num_edges() const {
            return m_num_edges;
        }

        // Get node labels of graphs.
        const Labels& get_labels() const {
            return m_node_labels;
        }

        // Returns "true" if edge {u,w} exists, otherwise "false".
        bool has_edge(const Node v, const Node w) const {
            return find(m_adjacency_lists_outbound[v].begin(), m_adjacency_lists_outbound[v].end(), w) != m_adjacency_lists_outbound[v].end();
        }

      private:
        vector<vector<Node>> m_adjacency_lists_outbound;
        vector<vector<Node>> m_adjacency_lists_inbound;

        // Manage number of nodes in graph.
        size_t m_num_nodes;
        // Manage number of edges in graph.
        size_t m_num_edges;
        // Manage node labels.
        Labels m_node_labels;
        // Directed graph?
        const bool m_directed;
    };

    typedef vector<Graph> GraphDatabase;
}

#endif // GRAPH_H

