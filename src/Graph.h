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
#include <set>
#include <unordered_set>
#include <vector>

//using namespace std;

using uint = unsigned int;
using ulong = unsigned long;
using Node = uint;
using Nodes = std::vector<Node>;
using Label = ulong;
using Labels = std::vector<Label>;
using Edge = uint;
using Edges = std::vector<Edge>;

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
        // Ctors/dtor.
        Graph(bool directed = false)
          : m_num_nodes(0),
            m_directed(directed) {
        }
        Graph(const Graph &graph)
          : m_adjacency_lists_outbound(graph.m_adjacency_lists_outbound),
            m_adjacency_lists_inbound(graph.m_adjacency_lists_inbound),
            m_edges_src(graph.m_edges_src),
            m_edges_dst(graph.m_edges_dst),
            m_set_edge_labels(graph.m_set_edge_labels),
            m_edge_labels(graph.m_edge_labels),
            m_node_labels(graph.m_node_labels),
            m_num_nodes(graph.m_num_nodes),
            m_directed(graph.m_directed) {
        }
        Graph(Graph &&graph)
          : m_adjacency_lists_outbound(std::move(graph.m_adjacency_lists_outbound)),
            m_adjacency_lists_inbound(std::move(graph.m_adjacency_lists_inbound)),
            m_edges_src(std::move(graph.m_edges_src)),
            m_edges_dst(std::move(graph.m_edges_dst)),
            m_set_edge_labels(std::move(graph.m_set_edge_labels)),
            m_edge_labels(std::move(graph.m_edge_labels)),
            m_node_labels(std::move(graph.m_node_labels)),
            m_num_nodes(graph.m_num_nodes),
            m_directed(graph.m_directed) {
        }
        Graph(const uint num_nodes,
              const Labels node_labels,
              bool directed = false)
          : m_num_nodes(num_nodes),
            m_node_labels(node_labels),
            m_directed(directed) {
            m_adjacency_lists_outbound.resize(num_nodes);
            m_adjacency_lists_inbound.resize(num_nodes);
        }
        Graph(const uint num_nodes,
              const Nodes &edges_src,
              const Nodes &edges_dst,
              const Labels &edge_labels,
              const Labels node_labels,
              bool directed = false)
          : m_num_nodes(num_nodes),
            m_node_labels(node_labels),
            m_directed(directed) {
            m_adjacency_lists_outbound.resize(num_nodes);
            m_adjacency_lists_inbound.resize(num_nodes);
            assert(edges_src.size() == edges_dst.size());
            assert(edges_src.size() == edge_labels.size());
            for( size_t i = 0; i < edges_src.size(); ++i ) {
                Label label = edge_labels.at(i);
                add_edge(edges_src.at(i), edges_dst.at(i), label);
            }
        }
        Graph(const uint num_nodes,
              const std::vector<std::pair<Node, Node>> &edgeList,
              const Labels node_labels,
              bool directed = false)
          : m_num_nodes(num_nodes),
            m_node_labels(node_labels),
            m_directed(directed) {
            m_adjacency_lists_outbound.resize(num_nodes);
            m_adjacency_lists_inbound.resize(num_nodes);
            for( auto const &e : edgeList )
                add_edge(e.first, e.second);
        }
        ~Graph() { }

        // Add a single node to the graph.
        size_t add_node() {
            m_adjacency_lists_outbound.push_back(std::vector<Node>{ });
            m_adjacency_lists_inbound.push_back(std::vector<Node>{ });
            return m_num_nodes++;
        }

        // Add a single edge to the graph.
        void add_edge(const Node v, const Node w, Label label = 0) {
            size_t n = get_num_edges();
            m_adjacency_lists_outbound.at(v).push_back(n);
            m_adjacency_lists_inbound.at(w).push_back(n);
            m_edges_src.push_back(v);
            m_edges_dst.push_back(w);
            m_edge_labels.push_back(label);
            m_set_edge_labels.insert(label);

            if( !m_directed ) {
                size_t n = get_num_edges();
                m_adjacency_lists_outbound.at(w).push_back(n);
                m_adjacency_lists_inbound.at(v).push_back(n);
                m_edges_src.push_back(w);
                m_edges_dst.push_back(v);
                m_edge_labels.push_back(label);
            }
        }

        // Get src/dst for edge
        Node get_src(const Edge e) const {
            return m_edges_src.at(e);
        }
        Node get_dst(const Edge e) const {
            return m_edges_dst.at(e);
        }

        // Get degree of node "v".
        size_t get_degree(const Node v) const {
            return m_directed ? get_out_degree(v) + get_in_degree(v) : get_out_degree(v);
        }
        size_t get_in_degree(const Node v) const {
            return m_adjacency_lists_outbound.at(v).size();
        }
        size_t get_out_degree(const Node v) const {
            return m_adjacency_lists_inbound.at(v).size();
        }

        // Get incident edges at node "v".
        const Edges& get_outbound_edges(const Node v) const {
            return m_adjacency_lists_outbound.at(v);
        }
        const Edges& get_inbound_edges(const Node v) const {
            return m_adjacency_lists_inbound.at(v);
        }
        Edges get_incident_edges(const Node v) const {
            Edges incident_edges(get_outbound_edges(v));
            const Edges &inbound_edges = get_inbound_edges(v);
            incident_edges.insert(incident_edges.end(), inbound_edges.begin(), inbound_edges.end());
            return incident_edges;
        }

        // Get edge labels and their set.
        const std::set<Label>& get_set_edge_labels() const {
            return m_set_edge_labels;
        }
        const std::vector<Label>& get_edge_labels() const {
            return m_edge_labels;
        }

        // Get number of nodes in graph.
        size_t get_num_nodes() const {
            return m_num_nodes;
        }

        // Get number of edges in graph.
        size_t get_num_edges() const {
            return m_edges_src.size();
        }

        // Get node labels of graphs.
        const Labels& get_labels() const {
            return m_node_labels;
        }

        // Returns "true" if edge {u,w} exists, otherwise "false".
        bool has_edge(const Node v, const Node w) const {
            for( Edge e : m_adjacency_lists_outbound.at(v) ) {
                if( (get_src(e) == v) && (get_dst(e) == w) )
                    return true;
            }
            return false;
        }

        // Whether graph is directed/undirected
        bool directed() const {
            return m_directed;
        }
        bool undirected() const {
            return !directed();
        }

      private:
        std::vector<Edges> m_adjacency_lists_outbound;
        std::vector<Edges> m_adjacency_lists_inbound;
        std::vector<Node> m_edges_src;
        std::vector<Node> m_edges_dst;

        std::set<Label> m_set_edge_labels;
        Labels m_edge_labels;
        Labels m_node_labels;

        size_t m_num_nodes;
        const bool m_directed;
    };

    typedef std::vector<Graph> GraphDatabase;
}

#endif // GRAPH_H

