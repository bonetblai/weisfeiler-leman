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

#ifndef COLOR_REFINEMENT_H
#define COLOR_REFINEMENT_H

#include <iostream>
#include <unordered_map>
#include "Graph.h"

using Labeling = std::unordered_multimap<Label, Node>;

namespace ColorRefinement {
    class ColorRefinement {
      public:
        // Ctors/dtor.
        explicit ColorRefinement(const GraphLibrary::Graph &graph) : m_graph(graph) { }
        ~ColorRefinement() { }

        // Compute stable coloring of graph "m_graph".
        void compute_stable_coloring(std::unordered_set<Label> &node_colors,
                                     std::unordered_multimap<Label, Node> &colors_to_nodes,
                                     std::unordered_map<Node, Label> &node_to_color,
                                     const Labels &node_labels,
                                     uint num_edge_labels,
                                     const Labels &edge_labels) const;
        void compute_stable_coloring(std::unordered_set<Label> &node_colors,
                                     std::unordered_multimap<Label, Node> &colors_to_nodes,
                                     std::unordered_map<Node, Label> &node_to_color) const;

      private:
        // A graph.
        GraphLibrary::Graph m_graph;

        // Bijection to map two labels to a single label.
        Label pairing(const Label a, const Label b) const {
            return a >= b ? a * a + a + b : a + b * b;
            // a=0, b=0 => a^2 + a + b = 0 + 0 + 0 = 0
            // a=0, b=1 => a + b^2 = 0 + 1 = 1
            // a=1, b=0 => a^2 + a + b = 1 + 1 + 0 = 2
            // a=1, b=1 => a^2 + a + b = 1 + 1 + 1 = 3
            // a=0, b=2 => a + b^2 = 0 + 4 = 4
            // a=1, b=2 => a + b^2 = 0 + 4 = 5
            // a=2, b=0 => a^2 + a + b = 4 + 2 + 0 = 6
            // a=2, b=1 => a^2 + a + b = 4 + 2 + 1 = 7
            // a=2, b=2 => a^2 + a + b = 4 + 2 + 2 = 8
            // a=0, b=3 => a + b^2 = 0 + 9 = 9
            // a=1, b=3 => a + b^2 = 1 + 9 = 10
            // a=2, b=3 => a + b^2 = 2 + 9 = 11
            // a=3, b=0 => a^2 + a + b = 9 + 3 + 0 = 12
            // a=3, b=1 => a^2 + a + b = 9 + 3 + 1 = 13
            // a=3, b=2 => a^2 + a + b = 9 + 3 + 2 = 14
            // a=3, b=3 => a^2 + a + b = 9 + 3 + 3 = 15
            // a=0, b=4 => a + b^2 = 0 + 16 = 16
            // a=1, b=4 => a + b^2 = 1 + 16 = 17
            // a=2, b=4 => a + b^2 = 2 + 16 = 18
            // a=3, b=4 => a + b^2 = 3 + 16 = 19
            // a=4, b=0 => a^2 + a + b = 16 + 4 + 0 = 20
            // a=4, b=1 => a^2 + a + b = 16 + 4 + 1 = 21
            // a=4, b=2 => a^2 + a + b = 16 + 4 + 2 = 22
            // a=4, b=3 => a^2 + a + b = 16 + 4 + 3 = 23
            // a=4, b=4 => a^2 + a + b = 16 + 4 + 4 = 24
            // a=0, b=5 => a + b^2 = 0 + 25 = 25
            // a=1, b=5 => a + b^2 = 1 + 25 = 26
            // a=2, b=5 => a + b^2 = 2 + 25 = 27
            // a=3, b=5 => a + b^2 = 3 + 25 = 28
            // a=4, b=5 => a + b^2 = 4 + 25 = 29
            // a=5, b=0 => a^2 + a + b = 25 + 5 + 0 = 30
            // a=5, b=1 => a^2 + a + b = 25 + 5 + 1 = 31
            // a=5, b=2 => a^2 + a + b = 25 + 5 + 2 = 32
            // a=5, b=3 => a^2 + a + b = 25 + 5 + 3 = 33
            // a=5, b=4 => a^2 + a + b = 25 + 5 + 4 = 34
            // a=5, b=5 => a^2 + a + b = 25 + 5 + 5 = 35
            // a=0, b=6 => a + b^2 = 0 + 36 = 36
            // a=1, b=6 => a + b^2 = 1 + 36 = 37
            // a=2, b=6 => a + b^2 = 2 + 36 = 38
            // a=3, b=6 => a + b^2 = 3 + 36 = 39
            // a=4, b=6 => a + b^2 = 4 + 36 = 40
            // a=5, b=6 => a + b^2 = 5 + 36 = 41
            // a=6, b=0 => a^2 + a + b = 36 + 6 + 0 = 42
            // a=6, b=1 => a^2 + a + b = 36 + 6 + 1 = 43
            // a=6, b=2 => a^2 + a + b = 36 + 6 + 2 = 44
            // a=6, b=3 => a^2 + a + b = 36 + 6 + 3 = 45
            // a=6, b=4 => a^2 + a + b = 36 + 6 + 4 = 46
            // a=6, b=5 => a^2 + a + b = 36 + 6 + 5 = 47
            // a=6, b=6 => a^2 + a + b = 36 + 6 + 6 = 48
            // a=0, b=7 => a + b^2 = 0 + 49 = 49
            // a=1, b=7 => a + b^2 = 1 + 49 = 50
            // a=2, b=7 => a + b^2 = 2 + 49 = 51
            // a=3, b=7 => a + b^2 = 3 + 49 = 52
            // a=4, b=7 => a + b^2 = 4 + 49 = 53
            // a=5, b=7 => a + b^2 = 5 + 49 = 54
            // a=6, b=7 => a + b^2 = 6 + 49 = 55
            // a=7, b=0 => a^2 + a + b = 49 + 7 + 0 = 56
            // a=7, b=1 => a^2 + a + b = 49 + 7 + 1 = 57
            // a=7, b=2 => a^2 + a + b = 49 + 7 + 2 = 58
            // a=7, b=3 => a^2 + a + b = 49 + 7 + 3 = 59
            // a=7, b=4 => a^2 + a + b = 49 + 7 + 4 = 60
            // a=7, b=5 => a^2 + a + b = 49 + 7 + 5 = 61
            // a=7, b=6 => a^2 + a + b = 49 + 7 + 6 = 62
            // a=7, b=7 => a^2 + a + b = 49 + 7 + 7 = 63
        }
    };
}

#endif // COLOR_REFINEMENT_H

