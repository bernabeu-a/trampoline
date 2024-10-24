#!/usr/bin/env python3

from subprocess import call
# from dfa import DFA


def read(filename):
    """ Read the specified dfa from a file.
        A DFA can be saved with the 'save' function.
        @param filename the file in which the DFA is stored.
        @return the DFA from the file.
        """
    local_dict = locals()
    with open(filename, "r") as file:
        exec(compile(open(filename).read(), filename, 'exec'), globals(), local_dict)

    return local_dict["a"]

def save(dfa, filename):
    """ Save the specified dfa into a file.
        A DFA can be read with the 'read' function.
        @param dfa      the DFA to save
        @param filename the name of the file in which
            the automaton will be saved."""
    txt = "a = DFA.DFA(\"" + dfa.alphabet + "\")\n"
    for state in dfa.states:
        if state in dfa.finals:
            txt += "a.add_state(\"" + state + "\", True)\n"
        else:
            txt += "a.add_state(\"" + state + "\")\n"

    txt += "\na.init = \"" + dfa.init + "\"\n\n"

    for state in dfa.states:
        for (symbol, dst_state) in dfa.transitions[state]:
            txt += "a.add_transition(\"" + state + "\", \"" + symbol + "\", \"" + dst_state + "\")\n"

    with open(filename, "w") as file:
        file.write(txt)

def to_dot(dfa, name="Graph"):
    """ Returns a string corresponding to the specified DFA in DOT format.
        @param dfa  the DFA to be converted in DOT format.
        @param name the name of the automaton for the DOT file ("Graph")
            by default.
        @returns the automaton in DOT format."""
    ret = "digraph " + name + " {\n    rankdir=\"LR\";\n\n"
    ret += "    // States (" + str(len(dfa.states)) + ")\n"

    state_name = lambda s : "Q_" + str(dfa.states.index(s))

    # States
    ret += "    node [shape = point ];     __Qi__ // Initial state\n" # Initial state
    for state in dfa.states:
        ret += "    "
        if state in dfa.finals:
            ret += "node [shape=doublecircle]; "
        else:
            ret += "node [shape=circle];       "
        ret += state_name(state) + ' [label="' + state + '"];\n'

    # Transitions
    ret += "\n    // Transitions\n"
    ret += "    __Qi__ -> " + state_name(dfa.init) + "; // Initial state arrow\n"
    for state in dfa.states:
        for (symbol, dst_state) in dfa.transitions[state]:
            ret += "    " + state_name(state) + " -> " + state_name(dst_state) + ' [label="' + symbol + '"];\n'
    return ret + "}\n"

def to_png(dfa, filename=None, name="Graph"):
    """ Create the PNG image corresponding to the representation of the
        specified DFA in a file.
        The automaton is converted in DOT format and the command dot is called
        in order to generate the PNG.
        @param dfa      the DFA to be converted in PNG.
        @param name     the name of the graph.
        @param filename the name of the PNG file, use the name of the graph if
            not specified. """

    if filename is None:
        filename = name + ".png"

    tmp_file = filename + ".dot"
    with open(tmp_file, "w") as file:
        file.write(to_dot(dfa, name))

    call(("dot -Tpng " + tmp_file + " -o " + filename+".png").split(" "))
    #call(("rm " + tmp_file).split(" "))


def to_pdf(dfa, filename=None, name="Graph"):
    """ Create the graphical PDF representation of the specified DFA in a file.
        The automaton is converted in DOT format and the command dot is called
        in order to generate the PDF.
        @param dfa      the DFA to be converted in PDF.
        @param name     the name of the graph.
        @param filename the name of the PDF file, use the name of the graph if
            not specified. """

    if filename is None:
        filename = name + ".pdf"

    tmp_file = filename + ".tmp"
    with open(tmp_file, "w") as file:
        file.write(to_dot(dfa, name))

    call(("dot -Tpdf " + tmp_file + " -o " + filename).split(" "))
    call(("rm " + tmp_file).split(" "))

