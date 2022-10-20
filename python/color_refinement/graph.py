'''
The following code is a extended python version of Amenability_test by Christopher Morris.

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
 * Modified by Blai Bonet (c) 2022
 *********************************************************************/
'''

class Graph:
    def __init__(self, **kwargs):
        if 'graph' in kwargs:
            # construct a copy of given graph
            graph = kwargs['graph']
            self._adjacency_lists_outbound = graph._adjacency_lists_outbound
            self._adjacency_lists_inbound = graph.__adjacency_lists_inbound
            self._edges_src = graph._edges_src
            self._edges_dst = graph._edges_dst
            self._set_node_labels = graph._set_node_labels
            self._set_edge_labels = graph._set_edge_labels
            self._node_labels = graph._node_labels
            self._edge_labels = graph._edge_labels
            self._num_nodes = graph._num_nodes
            self._directed = graph._directed
        else:
            self._directed = kwargs['directed']
            self._num_nodes = kwargs['num_nodes']

            self._adjacency_lists_outbound = [ [] for _ in range(self._num_nodes) ]
            self._adjacency_lists_inbound = [ [] for _ in range(self._num_nodes) ]
            self._edges_src = []
            self._edges_dst = []
            self._set_node_labels = set()
            self._set_edge_labels = set()
            self._node_labels = None
            self._edge_labels = []

            # set node/edge labels
            if 'node_labels' in kwargs:
                self._node_labels = kwargs['node_labels']
                assert len(self._node_labels) == self._num_nodes
            else:
                self._node_labels = [0] * self._num_nodes

            # insert edges
            edge_labels = kwargs['edge_labels'] if 'edge_labels' in kwargs else None
            if 'edge_list' in kwargs:
                edge_list = kwargs['edge_list']
                assert edge_labels is None or len(edge_labels) == len(edge_list)
                for i, v, w in enumerate(edge_list):
                    label = edge_labels[i] if edge_labels is not None else 0
                    self.add_edge(v, w, label)
            elif 'edges_src' in kwargs and 'edges_dst' in kwargs:
                edges_src, edges_dst = kwargs['edges_src'], kwargs['edges_dst']
                assert len(edges_src) == len(edges_dst)
                assert edge_labels is None or len(edge_labels) == len(edges_src)
                for i in range(len(edges_src)):
                    label = edge_labels[i] if edge_labels is not None else 0
                    self.add_edge(edges_src[i], edges_dst[i], label)

        assert self._node_labels is None or len(self._node_labels) == self.get_num_nodes()
        assert self._edge_labels is None or len(self._edge_labels) == self.get_num_edges()

    # add node/edge
    def add_node(self, label = 0):
        self._adjacency_lists_outbound.append([])
        self._adjacency_lists_inbound.append([])
        self._node_labels.append(label)
        self._set_node_labels.add(label)
        self._num_nodes += 1
        return self._num_nodes

    def add_edge(self, v, w, label = 0):
        n = self.get_num_edges()
        self._adjacency_lists_outbound[v].append(n)
        self._adjacency_lists_inbound[w].append(n)
        self._edges_src.append(v)
        self._edges_dst.append(w)
        self._edge_labels.append(label)
        self._set_edge_labels.add(label)

        if not self._directed:
            n = self.get_num_edges()
            self._adjacency_lists_outbound[w].append(n)
            self._adjacency_lists_inbound[v].append(n)
            self._edges_src.append(w)
            self._edges_dst.append(v)
            self._edge_labels.append(label)

    # get src/dst for edge
    def get_src(self, e):
        return self._edges_src[e]
    def get_dst(self, e):
        return self._edges_dst[e]

    # get degree of node
    def get_degree(self, v):
        return self.get_in_degree(v) + self.get_out_degree(v) if self._directed else self.get_out_degree(v)
    def get_in_degree(self, v):
        return len(self._adjacency_lists_outbound[v])
    def get_out_degree(self, v):
        return len(self._adjacency_lists_inbound[v])

    # get incident edges at node
    def get_outbound_edges(self, v):
        return self._adjacency_lists_outbound[v]
    def get_inbound_edges(self, v):
        return self._adjacency_lists_inbound[v]
    def get_incident_edges(self, v):
        return self.get_outbound_edges(v) + self.get_inbound_edges(v) if self._directed else self.get_outbound_edges(v)

    # get number of nodes/edges
    def get_num_nodes(self):
        return self._num_nodes
    def get_num_edges(self):
        return len(self._edges_src)

    # get labels and their sets
    def get_set_node_labels(self):
        return self._set_node_labels
    def get_node_labels(self):
        return self._node_labels
    def get_set_edge_labels(self):
        return self._set_edge_labels
    def get_edge_labels(self):
        return self._edge_labels

    # returns True if edge {v,w} exists, otherwise False
    def has_edge(self, v, w):
        for e in self._adjacency_lists_outbound[v]:
            if self.get_src(e) == v and self.get_dst(e) == w:
                return True
        return False

    # whether graph is directed/undirected
    def directed():
        return self._directed
    def undirected():
        return not self._directed

