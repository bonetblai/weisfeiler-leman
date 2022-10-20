import argparse
from timeit import default_timer as timer
from pathlib import Path

from color_refinement import Graph, ColorRefinement


def parse_arguments():
    parser = argparse.ArgumentParser(description='My description.')
    parser.add_argument('filename', type=str, nargs='+', help='file containing graph')
    parser.add_argument('--debug', action='store_true', help='print some debugging info')
    parser.add_argument('--disable-chosen-labels', action='store_true', help="don't use chosen() atoms in input (if enabled, all labels are considered)")
    parser.add_argument('--normalize-colors', action='store_true', help='normalize node colors after each iteration to avoid overflows')
    parser.add_argument('--uniform-initial-coloring', action='store_true', help='force uniform initial node coloring')
    args = parser.parse_args()
    return args

def get_args(atom, debug=False):
    args = []
    pos, n, rbegin = atom.index('('), 0, 0
    for i in range(pos, len(atom)):
        if n == 0 and atom[i] == '(':
            n += 1
            rbegin = i + 1
        elif atom[i] == '(':
            if n == 1: rbegin = i
            n += 1
        elif n == 1 and (atom[i] == ',' or atom[i] == ')'):
            rlen = i - rbegin
            arg = atom[rbegin:rbegin+rlen]
            args.append(arg)
            if atom[i] == ',':
                rbegin = i + 1
            else:
                n -= 1
        elif atom[i] == ')':
            assert n > 0
            n -= 1
    assert n == 0
    if debug: print(f'get_args: atom=|{atom}|, args={args}')
    return args

def read_lp_graph(filename, uniform_initial_coloring=False, use_chosen_labels=False, debug=False):
    lines = []
    with filename.open('r') as fd:
        lines = [ line.strip('\n') for line in fd ]

        map_node, map_edge = dict(), dict()
        map_label, map_edge_labels = dict(), dict()
        map_color, chosen_labels = dict(), set()

        for line in lines:
            if line.startswith('node('):
                args = get_args(line, debug)
                assert len(args) == 1
                node = ','.join(args)
                assert node not in map_node
                map_node[node] = len(map_node)
            elif line.startswith('labelname('):
                args = get_args(line, debug)
                assert len(args) == 2
                index, label = *args,
                assert int(index) not in map_label
                map_label[int(index)] = label
            elif line.startswith('edge('):
                args = get_args(line, debug)
                assert len(args) == 1
                edge_args = get_args(args[0], debug)
                assert len(edge_args) == 2
                edge = tuple(edge_args)
                assert edge not in map_edge
                map_edge[edge] = len(map_edge)
            elif line.startswith('tlabel('):
                args = get_args(line, debug)
                assert len(args) == 2
                label = int(args[1])
                edge_args = get_args(args[0], debug)
                assert len(edge_args) == 2
                edge = tuple(edge_args)
                if edge not in map_edge_labels:
                    map_edge_labels[edge] = []
                map_edge_labels[edge].append(label)
            elif line.startswith('chosen('):
                args = get_args(line, debug)
                assert len(args) == 1
                chosen_labels.add(int(args[0]))
            elif line.startswith('color('):
                args = get_args(line, debug)
                assert len(args) == 2
                node, color = args[0], int(args[1])
                map_color[node] = color

    num_nodes = len(map_node)
    num_edges = len(map_edge)
    num_labels = len(map_label)
    print(f'graph: #nodes={num_nodes}, #edges={num_edges}, #edge-labels={num_labels}')

    # check consistency
    for v, w in map_edge:
        assert v in map_node and w in map_node
    for v, w in map_edge_labels:
        assert v in map_node and w in map_node
        for label in map_edge_labels[(v, w)]:
            assert label in map_label

    # normalize edge labels
    remap_label = dict()
    for label in map_label:
        remap_label[label] = len(remap_label)

    # setup (initial) node colors (if specified), otherwise initial coloring is uniform
    node_labels = [1 for _ in range(num_nodes)]
    if not uniform_initial_coloring:
        for v in map_color:
            node = map_node[v]
            color = map_color[v]
            node_labels[node] = color

    # setup edges
    edges_src, edges_dst, edge_labels = [], [], []
    for v, w in map_edge:
        labels = map_edge_labels[(v, w)]
        assert len(labels) == 1
        label = labels[0]
        if not use_chosen_labels or label in chosen_labels:
            edges_src.append(map_node[v])
            edges_dst.append(map_node[w])
            edge_labels.append(remap_label[label])

    # construct and return graph
    kwargs = {
        'directed': True,
        'num_nodes': num_nodes,
        'node_labels': node_labels,
        'edges_src': edges_src,
        'edges_dst': edges_dst,
        'edge_labels': edge_labels,
    }
    return Graph(**kwargs)

def main(args):
    for filename in args.filename:
        graph = read_lp_graph(Path(filename),
                              uniform_initial_coloring=args.uniform_initial_coloring,
                              use_chosen_labels=not args.disable_chosen_labels,
                              debug=args.debug)
        cr = ColorRefinement(graph)

        # remap edge labels so that they fall in { 0, ..., num_edge_labels - 1 }
        edge_labels = graph.get_edge_labels()
        map_edge_label = dict()
        for e, label in enumerate(edge_labels):
            if label not in map_edge_label:
                map_edge_label[label] = len(map_edge_label)
            edge_labels[e] = map_edge_label[label]
        assert len(map_edge_label) == len(graph.get_set_edge_labels()), f'{len(map_edge_label)} {len(graph.get_set_edge_labels())}'

        # compute stable coloring
        start_time = timer()
        kwargs = {
            'node_labels': graph.get_node_labels(),
            'edge_labels': edge_labels,
            'num_edge_labels': len(map_edge_label),
            'normalize_colors': args.normalize_colors
        }
        num_iterations, node_colors, color_to_nodes, node_to_color = cr.compute_stable_coloring(**kwargs)
        elapsed_time = timer() - start_time
        print(f'WL: #iterations={num_iterations}, #colors={len(node_colors)}, elapsed_time={elapsed_time}')

        # print summary of coloring
        total = 0
        for color in node_colors:
            n = len(color_to_nodes[color])
            total += n
            print(f"{n} node(s) with color '{color}':", end='')
            for v in color_to_nodes[color]:
                print(f' {v}', end='')
            print('')
        print(f'total {total} node(s)')

        # print node coloring
        for v, color in enumerate(node_to_color):
            print(f'color({v},{color})')


if __name__ == '__main__':
    args = parse_arguments()
    main(args)

