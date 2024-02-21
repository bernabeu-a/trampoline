#!/usr/bin/env python3

import copy

class DFA:
    """ This class represent any type of deterministic finite automaton."""
    def __init__(self, alphabet):
        """ Initialise the finite automaton.
            @param the alphabet of the automaton."""

        """ List of string corresponding to states name.
            States are always identificated by name."""
        self.states = []
        """ Dictionary using src state as key and mapping it to a list of
            pair (dest_state, symbol)."""
        self.transitions = {}
        """ The string corresponding to the name of the initial state."""
        self.init = None
        """ A list containing the name of the final states."""
        self.finals = []
        """ A string containing all symbols in the alphabet."""
        self.alphabet = []
        for s in alphabet:
            if s not in self.alphabet:
                self.alphabet.append(s)

    def get_alphabet(self):
        return self.alphabet

    def get_states(self):
        return self.states
    
    def get_transitions(self):
        return self.transitions
        
    def add_symbol(self, symbol):
        if symbol not in self.alphabet:
            self.alphabet.append(symbol)

    def add_state(self, state, final = False):
        """ Add a new state. Print error if the state already exists.
            @param state    the name of the new state.
            @param final    a boolean determining if the state is
                final"""
        if state in self.states:
            print("error : state '" + state + "' already exists.")
            return
        self.transitions[state] = []
        self.states.append(state)
        if final:
            self.finals.append(state)
    
    def remove_state(self, state, final=False):
        """ Remove a state
            @param state    the name of the state to remove        
        """
        if state not in self.states:
            print("error : state '" + state + "' does not exists.")
            return
        self.states.remove(state)
        del self.transitions[state]
        list_transition_to_del = []
        for item_key, item_value in self.transitions.items():
            # print(item_key)
            for item_tuple in item_value:
                for item in item_tuple:
                    if item == state:
                        list_transition_to_del.append(item_tuple)
        for item in list_transition_to_del:
            for item_key, item_value in self.transitions.items():
                for item_tuple in item_value:
                    if item == item_tuple:
                        item_value.remove(item)
        if final:
            self.states.remove(state)

    def remove_transition(self, src_state, dst_state, symbol):
        """ Remove a transition
            @param src_state    the source state of the transition
            @param dst_state    the destination state of the transition
            @param symbol       the symbol of the transition
        """
        if symbol not in self.alphabet:
            print("error : transition '" + symbol + "' does not exists.")
            return
        tuple_to_del = (symbol, dst_state)
        for item_key, item_value in self.transitions.items():
            if item_key == src_state:
                for item_tuple in item_value:
                    if item_tuple == tuple_to_del:
                        item_value.remove(item_tuple)
        
    def valid_symbol(self, symbol):
        """ Returns true if the symbol is part of the alphabet,
            false otherwise.
            @param symbol the symbol to be tested """
        if symbol not in self.alphabet: return False
        return True

    def dst_state(self, src_state, symbol):
        """ Search the transition corresponding to the specified source state
            and symbol and returns the destination state. If the transition does
            not exists, return None.
            @param src_state the source state of the transition.
            @param symbol the symbol of the transition. """
        if src_state not in self.states:
            print("error : the state '" + src_state + "' is not an existing state.")
            return
        for (s, dst_state) in self.transitions[src_state]:
            if s == symbol:
                return dst_state
        return None

    def add_transition(self, src_state, symbol, dst_state):
        """ Add a transition to the FA. Print error if the automaton already have a
            transition for the specified source state and symbol.
            @param src_state the name of the source state.
            @param symbol the symbol of the transition
            @param dst_state the name of the destination state."""
        if not self.valid_symbol(symbol):
            print("error : the symbol '" + symbol + "' is not part of the alphabet.")
            return
        if src_state not in self.states:
            print("error : the state '" + src_state + "' is not an existing state.")
            return
        if dst_state not in self.states:
            print("error : the state '" + dst_state + "' is not an existing state.")
            return

        # if self.dst_state(src_state, symbol) != None:
        #     print("error : the transition (" + src_state + ", " + symbol + ", ...) already exists.")
        #     return

        self.transitions[src_state].append((symbol, dst_state))
        return

    def clone(self):
        """ Returns a copy of the DFA."""
        a = DFA(self.alphabet)
        a.states = self.states.copy()
        a.init = self.init
        a.finals = self.finals
        a.transitions = copy.deepcopy(self.transitions)
        return a

    def __str__(self):
        ret = "FA :\n"
        ret += "   - alphabet   : '" + self.alphabet + "'\n"
        ret += "   - init       : " + str(self.init) + "\n"
        ret += "   - finals     : " + str(self.finals) + "\n"
        ret += "   - states (%d) :\n" % (len(self.states))
        for state in self.states:
            ret += "       - (%s)" % (state)
            if len(self.transitions[state]) is 0:
                ret += ".\n"
            else:
                ret += ":\n"
                for (sym, dest) in self.transitions[state]:
                    ret += "          --(%s)--> (%s)\n" % (sym, dest)
        return ret

