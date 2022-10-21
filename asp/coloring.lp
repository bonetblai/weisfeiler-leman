% Constants
#const max_colors=20.

% Graph defined with atoms node/1, edge/1, labelname/2, and tlabel/2

% Define edge labels and colors
{ num_colors(1..max_colors) } = 1.
label(L) :- labelname(L,_).
color(1..K) :- num_colors(K).

% Assign colors to nodes
{ assign(S,C) : color(C) } = 1 :- node(S).

% Define eq/2 (resp. neq/2) for pair of nodes (resp. edges)
eq(S,T) :- node(S), node(T), assign(S,C), assign(T,C).
neq(S,T) :- edge((S,T)), assign(S,C), not assign(T,C).

% Color multiplicity of labels at nodes: number K of edges with label L that land on nodes of color C
multiplicity(S,L,C,K) :- node(S), label(L), color(C), { tlabel((S,T),L) : node(T), assign(T,C) } = K.

% Same multisets of edges modulo color
:- label(L), color(C), eq(S,T), S < T, multiplicity(S,L,C,K), not multiplicity(T,L,C,K).

% Break some symmetries
assign(0,1).

% Optimization: minimize colors
#minimize { K@1 : num_colors(K) }.

% Output
#show label/1.
#show num_colors/1.
#show assign/2.

