"""
main_logic: algorithms.py

This module contains the implementation of a class which stores
details about every algorithm (protocol) which is identified in
the source file
"""


class Algorithms(object):

    def __init__(self, upon_syntax, has_jumps, rank, snippets_to_add):
        super(object, self).__init__()
        self.__upon_syntax = upon_syntax
        self.__has_jumps = has_jumps
        self.__rank = rank

        # Used to duplicate code when phase jumps are detected
        self.__snippets_to_add = snippets_to_add

    @property
    def upon_syntax(self):
        return self.__upon_syntax

    @property
    def has_jumps(self):
        return self.upon_syntax

    @property
    def rank(self):
        return self.rank

    @property
    def snippets_to_add(self):
        return self.__snippets_to_add

    def update_with_snippets(self):
        """
        Take every jump node, determine and insert the jump
        snippets
        :return:
        """
        pass
