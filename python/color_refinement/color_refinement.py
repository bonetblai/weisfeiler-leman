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

# Use numpy's fixed-precision integers
import numpy as np

class ColorRefinement:
    def __init__(self, graph):
        self._graph = graph

    # coloring is done over graph with initial node colors and edge colors determined
    # by *arguments* node_labels and edge_labels in kwargs. Return values are set of
    # colors, and color_to_nodes and node_to_colors maps
    def compute_stable_coloring(self, **kwargs):
        # read optional arguments for node/edge labels
        node_labels = kwargs['node_labels'] if 'node_labels' in kwargs else [1] * self._graph.get_num_nodes()
        num_edge_labels = kwargs['num_edge_labels'] if 'num_edge_labels' in kwargs else 1
        edge_labels = kwargs['edge_labels'] if 'edge_labels' in kwargs else [0] * self._graph.get_num_edges()

        # assumption: edge labels are in { 0, ..., num_edge_labels - 1 }.
        for label in edge_labels: assert label < num_edge_labels

        # need node labels { 1, ... } since color 0 is marker
        add_one_to_node_labels = False
        for label in node_labels:
            if label == 0:
                add_one_to_node_labels = True
                break

        num_nodes = self._graph.get_num_nodes()
        assert len(node_labels) == num_nodes
        coloring = [ np.int64(0) for _ in range(num_nodes) ]
        num_old_colors = -1 # just to make it different from num_new_colors (below)

        # calculate new coloring using node labels
        new_colors = set()
        new_coloring = [ np.int64(label) for label in node_labels ]
        for v in range(num_nodes):
            if add_one_to_node_labels:
                new_coloring[v] += 1
            new_colors.add(new_coloring[v])
        num_new_colors = len(new_colors)

        while num_new_colors != num_old_colors:
            # update coloring
            coloring = new_coloring
            num_old_colors = num_new_colors
            new_colors.clear()

            # iterate over all nodes
            for v in range(num_nodes):
                # it is assumed that edge colors are in { 0, ..., num_edge_labels - 1 }
                outbound_edges = self._graph.get_outbound_edges(v)
                inbound_edges = self._graph.get_inbound_edges(v)
                colors = [ [] for _ in range(2 * num_edge_labels) ]

                # process edges by color whilte getting colors of neighbors
                for e in outbound_edges:
                    assert self._graph.get_src(e) == v
                    n = self._graph.get_dst(e)
                    edge_color = edge_labels[e]
                    colors[edge_color].append(coloring[n])
                for e in inbound_edges:
                    assert self._graph.get_dst(e) == v
                    n = self._graph.get_src(e)
                    edge_color = edge_labels[e]
                    colors[num_edge_labels + edge_color].append(coloring[n])

                # insert maker (color 0) in each group and sort them
                for i in range(num_edge_labels):
                    colors[i].append(0)
                    colors[num_edge_labels + i].append(0)
                    colors[i].sort()
                    colors[num_edge_labels + i].sort()

                # compute new label using composition to bijectively map two integers into
                # one integer. Initial color is color of vertex v as this must be taking
                # into account as well.
                new_color = coloring[v]
                for i in range(num_edge_labels):
                    for c in colors[i]:
                        new_color = self.pairing(new_color, c)
                    for c in colors[num_edge_labels + i]:
                        new_color = self.pairing(new_color, c)
                new_colors.add(new_color)
                new_coloring[v] = new_color
            num_new_colors = len(new_colors)

        # compute output
        node_colors = new_colors
        color_to_nodes, node_to_color = dict(), []
        for v in range(num_nodes):
            color = new_coloring[v]
            if color not in color_to_nodes:
                color_to_nodes[color] = set()
            color_to_nodes[color].add(v)
            node_to_color.append(color)
        return node_colors, color_to_nodes, node_to_color

    # pairing function: bijection of NxN onto N
    def pairing(self, a, b):
        r = a * a + a + b if a >= b else a + b * b
        r = -r if r < 0 else r
        #print(f'pairing: a={a}, b={b}, r={r}')
        return r

