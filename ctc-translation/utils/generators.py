"""
utils: generators.py

This module implements Visitors which extend either c_ast.NodeVisitor
or c_generator.CGenerator.

Visitors offer a simpler way to iterate through every node of the tree
but focus only on some of them.

Usual implementation: define methods for nodes on which you wan to focus
(visit_Assignment, ...) and use fields to return those nodes as a list or
different status.

Results form visitors will be handled in wrapper functions.
whileAlgoVisitor has more functionality as it implements different methods
which analyse an inner algorithm

"""


import copy
from pycparser.c_ast import While, Assignment, ID, If, Node, FuncDef, FileAST, Constant, UnaryOp, Compound, FuncCall, \
    Break, StructRef, BinaryOp, TypeDecl, PtrDecl, Decl, Struct, Enum
from pycparser import c_generator, c_ast


class LabelVisitor(c_ast.NodeVisitor):
    """
    Takes a label name and a label value as arguments and stores in a list (self.label_item)
    all the occurences (nodes) of this assignment operation.
    """

    def __init__(self, label_name, label_value):
        self.label_name = label_name
        self.label_value = label_value
        self.label_item = []

    def visit_Assignment(self, node):
        if not isinstance(node.lvalue, ID):
            return
        if node.lvalue.name == self.label_name:
            if isinstance(node.rvalue, ID):
                if node.rvalue.name == self.label_value:
                    self.label_item.append(node)
            elif isinstance(node.rvalue, Constant):
                if int(node.rvalue.value) == int(self.label_value):
                    self.label_item.append(node)


class EpochVisitor(c_ast.NodeVisitor):
    """
    Takes the name of the epoch variable and returns the list with all occurences
    """

    def __init__(self, epoch_name):
        self.epoch_name = epoch_name
        self.epoch_list = []

    def visit_Assignment(self, node):
        if node.lvalue.name == self.epoch_name:
            self.epoch_list.append(node)


class NullAssignVisitor(c_ast.NodeVisitor):
    """
    Takes the name of all message variables and mailbox variables and returns
    a list with all NULL assignments
    """

    def __init__(self, name_list):
        self.name_list = name_list
        self.result_list = []

    def visit_Assignment(self, node):
        if node.lvalue.name not in self.name_list:
            return

        if isinstance(node.rvalue, ID) and node.rvalue.name == "NULL":
            self.result_list.append(node)


class UselessFuncVisitor(c_ast.NodeVisitor):
    """
    Takes all free type functions and creates a list with them
    Also adds all functions which refer to a timeout, as these will
    be useless after recv whiles turning into ifs
    """

    def __init__(self, dispose_funcs_names):
        self.result_list = []
        self.dispose_funcs_names = dispose_funcs_names

    def visit_FuncCall(self, node):
        stat = False

        if node.name.name in self.dispose_funcs_names:
            stat = True

        if "clear" in node.name.name:
            stat = True

        if "clean" in node.name.name:
            stat = True

        if "free" in node.name.name:
            stat = True

        if "dispose" in node.name.name:
            stat = True

        if "delete" in node.name.name:
            stat = True

        if "erase" in node.name.name:
            stat = True

        if "timeout" in node.name.name:
            stat = True

        if stat:
            self.result_list.append(node)


class EmptyInstrVisitor(c_ast.NodeVisitor):
    """
    If there are instr with empty compounds,
    add them in a list and then delete them
    """

    def __init__(self):
        self.result_list = []

    def visit_If(self, node):
        if node.iffalse:
            if isinstance(node.iffalse, Compound) and len(node.iffalse.block_items) != 0:
                self.generic_visit(node.iffalse)

        if node.iffalse:
            if isinstance(node.iffalse, Compound) and len(node.iffalse.block_items) == 0:
                node.iffalse = None

        if node.iffalse:
            if isinstance(node.iffalse, If):
                self.visit_If(node.iffalse)
            elif isinstance(node.iffalse, While):
                self.visit_While(node.iffalse)
            elif not isinstance(node.iffalse, Compound):
                self.generic_visit(node.iffalse)

        if node.iftrue:
            if isinstance(node.iftrue, Compound) and len(node.iftrue.block_items) != 0:
                self.generic_visit(node.iftrue)

        if node.iftrue:
            if isinstance(node.iftrue, Compound) and len(node.iftrue.block_items) == 0:
                node.iftrue = None

        if node.iftrue:
            if isinstance(node.iftrue, If):
                self.visit_If(node.iftrue)
            elif isinstance(node.iftrue, While):
                self.visit_While(node.iftrue)
            elif not isinstance(node.iftrue, Compound):
                self.generic_visit(node.iftrue)

        if (not node.iftrue) and (not node.iffalse):
            self.result_list.append(node)
            return

        if (not node.iftrue) and node.iffalse:
            new_cond = UnaryOp("!", node.cond)
            node.cond = new_cond
            node.iftrue = node.iffalse
            node.iffalse = None

    def visit_While(self, node):
        if node.stmt:
            if isinstance(node.stmt, If):
                self.visit_If(node.stmt)
            elif isinstance(node.stmt, While):
                self.visit_While(node.stmt)
            elif not isinstance(node.stmt, Compound):
                self.generic_visit(node.stmt)

        if node.stmt:
            if isinstance(node.stmt, Compound) and len(node.stmt.block_items) != 0:
                self.generic_visit(node.stmt)

        if not node.stmt:
            self.result_list.append(node)
        elif isinstance(node.stmt, Compound) and len(node.stmt.block_items) == 0:
            self.result_list.append(node)


class RecvCallVisitor(c_ast.NodeVisitor):
    """
    With this visitor we can identify the number of recv type function
    calls from a certain while(true) block
    Used to decide whether a while loop is a receiving one, or a new
    algorithm
    """
    def __init__(self):
        self.nr = 0

    def visit_FuncCall(self, node):
        if ("recv" in str.lower(node.name.name)) or ("receive" in str.lower(node.name.name)):
            self.nr += 1

        if isinstance(node.args, FuncCall):
            self.visit_FuncCall(node.args)
        elif node.args:
            self.generic_visit(node.args)

    def visit_While(self, node):
        # If we find a while loop, this is either a new algo, or another recv loop
        # so we skip its content
        if WhileAlgoVisitor.check_while_cond(node.cond):
            return

        self.generic_visit(node.stmt)


class AssigRoundVisitor(c_ast.NodeVisitor):
    """
    This visitor checks if a block contains a round assignment
    If it contains and the block is a while (true), then it's
    considered to be a nested algo

    Also return the first assignment
    """
    def __init__(self, rounds_list, msg_fields):
        self.status = True
        self.rounds_list = rounds_list
        self.msg_fields = msg_fields
        self.assig = None

    def visit_Assignment(self, node):

        check1 = False
        if isinstance(node.lvalue, ID):
            for el in self.msg_fields:
                if node.lvalue.name == el["round_field"]:
                    check1 = True
                    break

        if not check1:
            return

        if node.rvalue and isinstance(node.rvalue, ID):
            for my_list in self.rounds_list:
                if node.rvalue.name in my_list:
                    self.status = False
                    if not self.assig:
                        self.assig = node
                    break


class AssigDummyVisitor(c_ast.NodeVisitor):
    """
    Create a list with all assignments to ERR_ROUND
    Then refine them in take_lab.py and delete the
    ones which are dummy
    """
    def __init__(self):
        self.result = []

    def visit_Assignment(self, node):

        if isinstance(node.rvalue, ID) and node.rvalue.name == "ERR_ROUND":
            self.result.append(node)


class JumpPhaseVisitor(c_ast.NodeVisitor):
    """
    Checks on a while loop algo if there is a
    phase jump - an assignment to phase different from
    phase + 1. phase++m phase-- and phase = phase + 1
    will be stored such that they can be modified and
    then printed in the synchronous code
    """
    def __init__(self, phase_var_name):
        self.__result = False
        self.__assig_list = []
        self.__incr_assig = []
        self.__incr_unary = []
        self.__phase_var_name = phase_var_name

    @property
    def incr_assig(self):
        return self.__incr_assig

    @property
    def incr_unary(self):
        return self.__incr_unary

    @property
    def assig_list(self):
        return self.__assig_list

    @property
    def phase_var_name(self):
        return self.__phase_var_name

    @property
    def result(self):
        return self.__result

    @result.setter
    def result(self, value):
        self.__result = value

    def visit_Assignment(self, node):

        if not (isinstance(node.lvalue, ID) and node.lvalue.name == self.phase_var_name):
            return

        if isinstance(node.rvalue, BinaryOp):
            if node.rvalue.op == "+":
                if isinstance(node.rvalue.left, ID) and node.rvalue.left.name == self.phase_var_name:
                    if isinstance(node.rvalue.right, Constant) and node.rvalue.right.value == "1":
                        self.incr_assig.append(node)
                        return

        self.result = True
        self.assig_list.append(node)

    def visit_UnaryOp(self, node):

        if node.op == "p++" or node.op == "p--":
            if isinstance(node.expr, ID) and node.expr.name == self.phase_var_name:
                self.incr_unary.append(node)



class WhileAlgoVisitor(c_ast.NodeVisitor):
    """
    With this visitor we return a list of whiles which represent
    a nested algorithm
    We also add markers for the beginning and ending of this algo
    """
    def __init__(self, rounds_list, msg_fields):
        self.result_list = []
        self.rounds_list = rounds_list
        self.msg_fields = msg_fields

    @staticmethod
    def check_if_recv_loop(node):
        # Checks if a node is a recv loop, by having
        # a recv() call top level
        v = RecvCallVisitor()
        v.visit(node)
        if v.nr > 0:
            return True

        return False

    @staticmethod
    def check_while_cond(cond):
        # determines if the while is a while true
        if isinstance(cond, ID) and cond.name == "true":
            return True

        if isinstance(cond, Constant) and cond.value == "1":
            return True

        return False

    @staticmethod
    def get_rank_of_algo(x, rounds_list, msg_structure_fields):
        # While loops will be either for receive either another algorithm
        # We are interested only in assignments which contain a round label
        # as right member

        if not x or (not isinstance(x, Compound)) or not x.block_items:
            return -1

        for elem in x.block_items:
            if isinstance(elem, Assignment):
                # First search to see if this is a round assignment
                found = False
                for struc in msg_structure_fields:
                    if elem.lvalue.name == struc['round_field']:
                        found = True
                        break

                if not found:
                    continue
                for mround in rounds_list:
                    if isinstance(elem.rvalue, ID) and elem.rvalue.name in mround:
                        return rounds_list.index(mround)
            else:
                v = -1
                if isinstance(elem, If) and elem.iftrue and isinstance(elem.iftrue, Compound):
                    v = WhileAlgoVisitor.get_rank_of_algo(elem.iftrue, rounds_list, msg_structure_fields)

                if v != -1:
                    return v

                if isinstance(elem, If) and elem.iffalse and isinstance(elem.iffalse, Compound):
                    v = WhileAlgoVisitor.get_rank_of_algo(elem.iftrue, rounds_list, msg_structure_fields)

                if v != -1:
                    return v

                if isinstance(elem, While) and WhileAlgoVisitor.check_while_cond(elem.cond):
                    # Check while recv for round assignments as well
                    if WhileAlgoVisitor.check_if_recv_loop(elem.stmt):
                        v = WhileAlgoVisitor.get_rank_of_algo(elem.stmt, rounds_list, msg_structure_fields)
                        if v != -1:
                            return v

        return -1

    def visit_While(self, node):
        if node.stmt:
            if isinstance(node.stmt, While):
                self.visit_While(node.stmt)
            else:
                self.generic_visit(node.stmt)

        if not WhileAlgoVisitor.check_while_cond(node.cond):
            return

        if WhileAlgoVisitor.check_if_recv_loop(node.stmt):
            return

        v = AssigRoundVisitor(self.rounds_list, self.msg_fields)
        v.visit(node)

        if v.status:
            return
        first_assig = v.assig

        self.result_list.append(node)


class DeclAlgoVisitor(c_ast.NodeVisitor):
    """
    This visitor creates a list with all variables declared inside an algorithm
    """

    def __init__(self, mbox_name):
        self.result_list = []
        self.mbox_name = mbox_name

    def visit_Typedef(self, node):
        return

    def visit_FuncDef(self, node):
        if node.decl.name != "main":
            return

        self.generic_visit(node.body)

    def visit_Decl(self, node):
        if len(node.storage) != 0:
            return

        if (not isinstance(node.type, TypeDecl)) and (not isinstance(node.type, PtrDecl)):
            return

        if isinstance(node.type, TypeDecl):
            inner_type = node.type.type

            if isinstance(inner_type, Struct):
                self.result_list.append((node.name, "struct " + inner_type.name))
            elif isinstance(inner_type, Enum):
                self.result_list.append((node.name, "enum " + inner_type.name))
            else:
                self.result_list.append((node.name, inner_type.names[0]))
        elif isinstance(node.type, PtrDecl):
            inner_type = node.type
            while isinstance(inner_type, PtrDecl):
                inner_type = inner_type.type
            inner_type = inner_type.type

            if isinstance(inner_type, Struct):
                self.result_list.append((node.name, "struct " + inner_type.name + "*"))
            else:
                self.result_list.append((node.name, inner_type.names[0] + "*"))

    def visit_Assignment(self, node):
        # If we have an assignment old_0_mbox = ... then
        # this is like a declared var
        if isinstance(node.lvalue, ID):
            if "old" in node.lvalue.name and self.mbox_name in node.lvalue.name:
                self.result_list.append((node.lvalue.name, ""))
        elif isinstance(node.lvalue, Decl):
            self.visit_Decl(node.lvalue)
        else:
            self.generic_visit(node.lvalue)

        if isinstance(node.rvalue, Decl):
            self.visit_Decl(node.rvalue)
        elif isinstance(node.rvalue, Assignment):
            self.visit_Assignment(node.rvalue)
        else:
            self.generic_visit(node.rvalue)


class AllVarsAlgoVisitor(c_ast.NodeVisitor):
    """
    This visitor creates a list with all IDs met inside an algo
    """

    def __init__(self):
        self.result_list = []

    def visit_ID(self, node):
        if node.name == "NULL":
            return

        if node.name == "to_all":
            return

        self.result_list.append(node.name)

    def visit_FuncCall(self, node):
        if "inner_algorithm" in node.name.name:
            return

        if node.args:
            if node.args.exprs:
                self.generic_visit(node.args.exprs)

    def visit_StructRef(self, node):
        if node.name:
            if isinstance(node.name, ID):
                self.result_list.append(node.name.name)


class CondVisitor(c_generator.CGenerator):

    def __init__(self, current_round, is_upon):
        c_generator.CGenerator.__init__(self)
        self.current_round = current_round
        self.inside_binop = False
        self.save_round = ""
        self.is_upon = is_upon

    def visit_ID(self, n):
        if not self.is_upon and self.inside_binop is False:
            if "==" in n.name and "_ROUND" in n.name and self.current_round not in n.name:
                return 'jump == True'

            if "!=" in n.name and "_ROUND" in n.name and self.current_round not in n.name:
                return 'jump != True'
        return n.name

    def visit_BinaryOp(self, n):
        changed = False
        if self.inside_binop is False:
            self.inside_binop = True
            changed = True

        if not self.is_upon and isinstance(n.right, ID):
            if n.right.name.endswith("_ROUND") and n.right.name != self.current_round:
                if changed:
                    self.inside_binop = False
                self.save_round = n.right.name
                return '%s %s %s' % ('jump', n.op, 'True')

        lval_str = self._parenthesize_if(n.left,
                            lambda d: not self._is_simple_node(d))
        rval_str = self._parenthesize_if(n.right,
                            lambda d: not self._is_simple_node(d))
        if changed:
            self.inside_binop = False
        return '%s %s %s' % (lval_str, n.op, rval_str)


class SendWhileVisitor(c_ast.NodeVisitor):
    """
    Returns a list with all occurences of send function
    """

    def __init__(self):
        self.list = []

    def visit_FuncCall(self, node):
        if node.name.name == 'send':
            self.list.append(node)


class SendLoopsVisitor(c_ast.NodeVisitor):
    """
    All while(true) send loops identified and returned
    as a list
    """

    def __init__(self, rounds_list, msg_fields):
        self.result_list = []
        self.rounds_list = rounds_list
        self.msg_fields = msg_fields

    def visit_While(self, node):
        if node.stmt:
            if isinstance(node.stmt, While):
                self.visit_While(node.stmt)
            else:
                self.generic_visit(node.stmt)

        if not WhileAlgoVisitor.check_while_cond(node.cond):
            return

        if WhileAlgoVisitor.check_if_recv_loop(node.stmt):
            return

        v = AssigRoundVisitor(self.rounds_list, self.msg_fields)
        v.visit(node)

        if not v.status:
            return

        v = SendWhileVisitor()
        v.visit(node)

        if len(v.list) > 0:
            self.result_list.append(node)


class TimeoutVisitor(c_ast.NodeVisitor):
    """
    Finds if there is an algorithm exit from a recv while loop
    Exists are represented by the call to out_internal()
    """
    def __init__(self):
        self.status = False
    """
    def visit_If(self, node):
        if isinstance(node.cond, FuncCall):
            if "timeout" in node.cond.name.name:
                if isinstance(node.iftrue, FuncCall) and node.iftrue.name.name == "out_internal":
                    self.status = True
                elif isinstance(node.iftrue, Compound):
                    self.generic_visit(node.iftrue)

        if self.status:
            return

        if node.iftrue:
            if isinstance(node.iftrue, If):
                self.visit_If(node.iftrue)
            else:
                self.generic_visit(node.iftrue)

        if node.iffalse:
            if isinstance(node.iffalse, If):
                self.visit_If(node.iffalse)
            else:
                self.generic_visit(node.iffalse)
    """
    def visit_FuncCall(self, node):
        if node.name.name == "out_internal":
            self.status = True


class RecvWhileVisitor(c_ast.NodeVisitor):
    """
    Identifies the recv whiles, used for asserts

    Returns a dictionary, result, with data regarding
    the return from the algorithm which contains this
    loop. Also contains the rank of the algorithm which
    contains this loop.

    Also returns a dictionary with each recv loop,
    for further analysis, including the index of the
    algorithm which contains it

    Separation for those which contain an out_internal, as they need to call
    return_from_inner at some point
    """

    def __init__(self, msg_fields, rounds_list):
        super(RecvWhileVisitor, self).__init__()
        self.result = {}
        self.recv_loops = {}
        self.algo_list = []
        self.msg_fields = msg_fields
        self.rounds_list = rounds_list

    def visit_While(self, n):
        algo_check = True
        if not WhileAlgoVisitor.check_while_cond(n.cond):
            algo_check = False

        if WhileAlgoVisitor.check_if_recv_loop(n.stmt):
            algo_check = False

        prev = len(self.algo_list)
        if algo_check:
            rank = WhileAlgoVisitor.get_rank_of_algo(n.stmt, self.rounds_list, self.msg_fields)

            if rank != -1:
                self.algo_list.append(rank)

        if not prev == len(self.algo_list) - 1:
            algo_check = False

        if n.stmt:
            if isinstance(n.stmt, While):
                self.visit_While(n.stmt)
            else:
                self.generic_visit(n.stmt)

        if algo_check:
            del self.algo_list[len(self.algo_list) - 1]

        if not WhileAlgoVisitor.check_while_cond(n.cond):
            return

        if WhileAlgoVisitor.check_if_recv_loop(n.stmt):
            self.recv_loops[n] = self.algo_list[-1]
            if len(self.algo_list) == 1:
                self.result[(n.coord.line, n.coord.column)] = (None, self.msg_fields[self.algo_list[0]]["round_field"],
                                                               self.algo_list[0])
                return

            # Check if it contains if (timeout()) {out_internal()}
            v = TimeoutVisitor()
            v.visit(n)

            if v.status:
                assig_node = Assignment('=', ID(self.msg_fields[self.algo_list[-2]]["round_field"]),
                                        ID(self.rounds_list[self.algo_list[-2]][0]), n.coord)
                self.result[(n.coord.line, n.coord.column)] = (assig_node,
                                                               self.msg_fields[self.algo_list[-1]]["round_field"],
                                                               self.algo_list[-2])


class CheckLabelNumber(c_ast.NodeVisitor):
    """
    Takes a label name as argument and checks the number of label assignments in a AST.
    Stores this number in self.count_labels
    """

    def __init__(self, label_name):
        self.label_name = label_name
        self.count_labels = 0

    def visit_Assignment(self, node):
        if node.lvalue.name == self.label_name:
            self.count_labels += 1


class SwitchToOldVars(c_ast.NodeVisitor):
    """
    This visitor will be used in the UPON syntax on every if.cond found. It switches
    variables to old names. Variables will be given as a list in the constructor

    Also it inserts old variables before the assignment of the next round. IDs from
    cond will be modified only if they are children of an if statement.

    After the initial assignment, if.cond IDs won't be visited as they need to be the new
    ones (they will follow after while recv). The ast is given so that we cna insert the old vars
    assignments in the parent of the assignment
    """

    def __init__(self, vars_list, round_label, round_name):
        self.vars_list = vars_list
        self.round_label = round_label
        self.round_name = round_name
        self.in_if = False
        self.found = False
        self.other_assigs = []

    def visit_ID(self, node):
        if not self.in_if:
            return

        if self.found:
            return

        if node.name in self.vars_list:
            node.name = "old_" + node.name

    def visit_If(self, node):
        self.in_if = True

        self.generic_visit(node.cond)

        if node.iftrue:
            if isinstance(node.iftrue, If):
                self.visit_If(node.iftrue)
                self.in_if = True
            else:
                self.generic_visit(node.iftrue)

        if node.iffalse:
            if isinstance(node.iffalse, If):
                self.visit_If(node.iffalse)
                self.in_if = True
            else:
                self.generic_visit(node.iffalse)

        self.in_if = False

    def visit_Assignment(self, node):
        if isinstance(node.lvalue, ID) and node.lvalue.name == self.round_label:
            if isinstance(node.rvalue, ID) and node.rvalue.name == self.round_name:
                self.found = True
                return

        if isinstance(node.lvalue, ID) and node.lvalue.name == self.round_label:
            self.other_assigs.append(node)

    def visit_StructRef(self, node):
        if not self.in_if:
            return

        if self.found:
            return
        if isinstance(node.name, ID):
            node.name.name = "old_" + node.name.name

class LocateParentNode(c_generator.CGenerator):
    """
    Takes a node as argument and find his parent in the AST.
    The parent is stored in self.discovered_node
    """

    def __init__(self, node):
        c_generator.CGenerator.__init__(self)
        self.node_to_find = node
        self.discovered_node = None

    def visit_Compound(self, n):
        if self.node_to_find == n:
            self.discovered_node = n
        s = ''
        if n.block_items:
            for stmt in n.block_items:
                if stmt == self.node_to_find:
                    self.discovered_node = n
                    break
                self._generate_stmt(stmt)
        return s


class LocateParentNodeUpdated(c_generator.CGenerator):
    """
    Takes a node as argument and find his parent in the AST.
    The parent is stored in self.discovered_node
    """

    def __init__(self, node):
        c_generator.CGenerator.__init__(self)
        self.node_to_find = node
        self.discovered_node = None

    def visit_If(self, n):
        s = ''
        if n.iftrue:
            if n.iftrue == self.node_to_find:
                self.discovered_node = n
            elif isinstance(n.iftrue, Compound):
                for stmt in n.iftrue.block_items:
                    if stmt == self.node_to_find:
                        self.discovered_node = n.iftrue
                        break
                    self._generate_stmt(stmt)
        if n.iffalse:
            if n.iffalse == self.node_to_find:
                self.discovered_node = n
            elif isinstance(n.iffalse, Compound):
                for stmt in n.iffalse.block_items:
                    if stmt == self.node_to_find:
                        self.discovered_node = n.iffalse
                        break
                    self._generate_stmt(stmt)

        return s

    def visit_While(self, n):
        s = ''
        self._generate_stmt(n.stmt)

        if n.stmt == self.node_to_find:
            self.discovered_node = n
        return s

    def visit_Compound(self, n):
        s = ''
        if n.block_items:
            for stmt in n.block_items:
                if stmt == self.node_to_find:
                    self.discovered_node = n
                    break
                self._generate_stmt(stmt)
        return s


class LocateNode(c_generator.CGenerator):
    """
    Takes a node as argument and finds that node in a tree
    """

    def __init__(self, node):
        c_generator.CGenerator.__init__(self)
        self.node_to_find = node
        self.discovered_node = None

    def visit_If(self, n):
        if self.node_to_find == n:
            self.discovered_node = n
        s = ''
        if n.cond: s += self.visit(n.cond)
        if n.iftrue:
            s += self._generate_stmt(n.iftrue, add_indent=True)
        if n.iffalse:
            s += self._generate_stmt(n.iffalse, add_indent=True)
        return s

    def visit_Compound(self, n):
        if self.node_to_find == n:
            self.discovered_node = n
        s = ''
        if n.block_items:
            s += ''.join(self._generate_stmt(stmt) for stmt in n.block_items)
        return s

    def visit_Break(self, n):
        if self.node_to_find == n:
            self.discovered_node = n
        return 'break;'

    def visit_While(self, n):
        s = ''
        if self.node_to_find == n:
            self.discovered_node = n

        if n.cond: s += self.visit(n.cond)
        s += self._generate_stmt(n.stmt, add_indent=True)
        return s

    def visit_BinaryOp(self, n):
        if self.node_to_find == n:
            self.discovered_node = n
        return ''

    def visit_Assignment(self, n):
        if self.node_to_find == n:
            self.discovered_node = n
        return ''

    def visit_FuncCall(self, n):
        if self.node_to_find == n:
            self.discovered_node = n
        return ''

    def visit_UnaryOp(self, n):
        if self.node_to_find == n:
            self.discovered_node = n
        return ''

    def visit_Constant(self, n):
        if self.node_to_find == n:
            self.discovered_node = n
        return ''

    def visit_ID(self, n):
        if self.node_to_find == n:
            self.discovered_node = n
        return ''

    def visit_Decl(self, n, no_type=False):
        if self.node_to_find == n:
            self.discovered_node = n
        return ''

    def visit_DeclList(self, n):
        if self.node_to_find == n:
            self.discovered_node = n
        return ''

    def visit_For(self, n):
        if self.node_to_find == n:
            self.discovered_node = n
        return ''


class TreeGenerator(c_generator.CGenerator):
    """
    To be used when printing a tree. Is better than the default pycparser generator
    because this one prints the else branch as if (!( )) and this is useful after modifying the tree.
    """

    def __init__(self):
        c_generator.CGenerator.__init__(self)

    def visit_If(self, n):
        s = 'if ('
        if n.cond:
            if n.iffalse is not None and n.iftrue is None:
                s += '!('
            s += self.visit(n.cond)
            if n.iffalse is not None and n.iftrue is None:
                s += ')'
        s += ')\n'
        if n.iftrue:
            s += self._generate_stmt(n.iftrue, add_indent=True)
        if n.iffalse:
            if n.iftrue is not None:
                s += self._make_indent() + 'else\n'
            s += self._generate_stmt(n.iffalse, add_indent=True)
        return s


class PathGenerator(c_generator.CGenerator):
    """
    Takes a path, i.e., a list of nodes as argument and generates the code from it
    by visiting the tree and printing only the nodes that are in the path list.
    """

    def __init__(self, path):
        c_generator.CGenerator.__init__(self)
        self.path = path
        # is set to True when you want to extend the search to nodes that are also in the path but are not
        # in the path list as singular nodes because they are part of bigger nodes that are also
        # in path but also included in path list as singular entities
        self.visit_condition = False
        self.extend_visit = False

    def visit_Compound(self, n):
        if n in self.path or self.extend_visit:
            s = self._make_indent() + '{\n'
            self.indent_level += 2
            if n.block_items:
                s += ''.join(
                    self._generate_stmt(stmt) for stmt in n.block_items if stmt in self.path or self.extend_visit)
            self.indent_level -= 2
            s += self._make_indent() + '}\n'
            return s
        else:
            return ''

    def visit_Assignment(self, n):
        if n in self.path or self.extend_visit:
            changed_value = False
            if self.extend_visit is False:
                self.extend_visit = True
                changed_value = True
            rval_str = self._parenthesize_if(
                n.rvalue,
                lambda n: isinstance(n, c_ast.Assignment))
            s = '%s %s %s' % (self.visit(n.lvalue), n.op, rval_str)
            if changed_value:
                self.extend_visit = False
            return s
        return ''

    def visit_If(self, n):
        if n in self.path or self.extend_visit:
            s = 'if ('
            if n.cond:
                if n.iffalse is not None and n.iffalse in self.path:
                    s += '!('
                self.visit_condition = True
                s += self.visit(n.cond)
                self.visit_condition = False
                if n.iffalse is not None and n.iffalse in self.path:
                    s += ')'
            s += ')\n'

            if (n.iftrue and n.iftrue not in self.path and not n.iffalse) or \
                    (n.iftrue and n.iffalse and n.iftrue not in self.path and n.iffalse not in self.path):
                self.extend_visit = True
                s += self._generate_stmt(n.iftrue, add_indent=True)
                if n.iffalse:
                    s += self._make_indent() + 'else\n'
                    s += self._generate_stmt(n.iffalse, add_indent=True)
                self.extend_visit = False
            else:
                if n.iftrue in self.path:
                    s += self._generate_stmt(n.iftrue, add_indent=True)
                else:
                    if n.iffalse:
                        s += self._generate_stmt(n.iffalse, add_indent=True)
            return s
        return ''

    def visit_While(self, n):
        s = 'int main ('
        s += ')\n'
        s += self._generate_stmt(n.stmt, add_indent=True)
        return s

    def visit_Decl(self, n, no_type=False):
        if n in self.path or self.extend_visit:
            changed_value = False
            if self.extend_visit is False:
                self.extend_visit = True
                changed_value = True
            s = n.name if no_type else self._generate_decl(n)
            if n.bitsize: s += ' : ' + self.visit(n.bitsize)
            if n.init:
                s += ' = ' + self._visit_expr(n.init)
            if changed_value:
                self.extend_visit = False
            return s
        return ''

    def visit_FuncCall(self, n):
        if n in self.path or self.visit_condition or self.extend_visit:
            changed_value = False
            if self.extend_visit is False:
                self.extend_visit = True
                changed_value = True
            fref = self._parenthesize_unless_simple(n.name)
            s = fref + '(' + self.visit(n.args) + ')'
            if changed_value:
                self.extend_visit = False
            return s
        return ''

    def visit_UnaryOp(self, n):
        if n in self.path or self.visit_condition or self.extend_visit:
            changed_value = False
            if self.extend_visit is False:
                self.extend_visit = True
                changed_value = True
            operand = self._parenthesize_unless_simple(n.expr)
            if n.op == 'p++':
                s = '%s++' % operand
                if changed_value:
                    self.extend_visit = False
                return s
            elif n.op == 'p--':
                s = '%s--' % operand
                if changed_value:
                    self.extend_visit = False
                return s
            elif n.op == 'sizeof':
                # Always parenthesize the argument of sizeof since it can be
                # a name.
                s = 'sizeof(%s)' % self.visit(n.expr)
                if changed_value:
                    self.extend_visit = False
                return s
            else:
                s = '%s%s' % (n.op, operand)
                if changed_value:
                    self.extend_visit = False
                return s
        return ''

    def visit_BinaryOp(self, n):
        if n in self.path or self.visit_condition or self.extend_visit:
            changed_value = False
            if self.extend_visit is False:
                self.extend_visit = True
                changed_value = True
            lval_str = self._parenthesize_if(n.left,
                                             lambda d: not self._is_simple_node(d))
            rval_str = self._parenthesize_if(n.right,
                                             lambda d: not self._is_simple_node(d))
            s = '%s %s %s' % (lval_str, n.op, rval_str)
            if changed_value:
                self.extend_visit = False
            return s
        return ''

    def visit_For(self, n):
        if n in self.path or self.visit_condition or self.extend_visit:
            changed_value = False
            if self.extend_visit is False:
                self.extend_visit = True
                changed_value = True
            s = 'for ('
            if n.init: s += self.visit(n.init)
            s += ';'
            if n.cond: s += ' ' + self.visit(n.cond)
            s += ';'
            if n.next: s += ' ' + self.visit(n.next)
            s += ')\n'
            s += self._generate_stmt(n.stmt, add_indent=True)
            if changed_value:
                self.extend_visit = False
            return s
        return ''


class RoundGenerator(c_generator.CGenerator):
    """
    This generator has two modes of work: send mode and update mode.
    In send mode, is generates the SEND() part of a round.
    In update mode, is generates the UPDATE() part of a round.
    It can generate a round either by using only the tree (this is the case when we don't have
    jumps of rounds or blocking while loops so we don't need to call find_all_paths function)
    - this is why the parameter path=None by default - or by using a tree and a path.
    The last case needs more complex computation.
    """

    def __init__(self, mode, labelname, current_round, delete_round_phase, message, variables, save_round, is_upon,
                 path=None):
        c_generator.CGenerator.__init__(self)
        # a string that indicates send or update mode; "send" for send and "update" for update
        self.mode = mode
        # reached send operation, can begin to save operations for update phase
        self.send_reached = False
        # if the send operation is the last one inside one inside an if body
        # then don't save that if on the way to update code
        self.send_last_instr = False
        # save if condition on the way to update phase, even if it is before the send operation
        self.visit_cond = False
        # first compound, i.e. the biggest while loop, don't print parentheses for this
        self.first_compound = True
        # the path to be checked
        self.path = path
        # extend powers to visit the whole element
        self.extend_visit = False
        # label name
        self.labelname = labelname
        # current round
        self.current_round = current_round
        # what to delete
        self.delete = delete_round_phase
        self.message = message
        self.variables = variables
        self.remember_round = save_round

        # is_upon determines if the current code has UPON syntax, which means a different strategy
        self.is_upon = is_upon

    def visit_Break(self, n):
        return 'out();'

    def visit_Decl(self, n, no_type=False):
        if self.path is not None:
            if n in self.path or self.extend_visit or self.visit_cond:
                if self.mode == "send" and not self.send_reached \
                        or self.mode == "update" and self.send_reached \
                        or self.visit_cond or self.extend_visit:
                    changed_value = False
                    if self.extend_visit is False:
                        self.extend_visit = True
                        changed_value = True
                    s = n.name if no_type else self._generate_decl(n)
                    if n.bitsize: s += ' : ' + self.visit(n.bitsize)
                    if n.init:
                        s += ' = ' + self._visit_expr(n.init)
                    if changed_value:
                        self.extend_visit = False
                    return s
        else:
            if self.mode == "send" and not self.send_reached \
                    or self.mode == "update" and self.send_reached \
                    or self.visit_cond:
                s = n.name if no_type else self._generate_decl(n)
                if n.bitsize: s += ' : ' + self.visit(n.bitsize)
                if n.init:
                    s += ' = ' + self._visit_expr(n.init)
                return s
        return ''

    def visit_FuncCall(self, n):
        if self.path is not None:
            if n in self.path or self.extend_visit or self.visit_cond:
                if self.mode == "send" and not self.send_reached \
                        or self.visit_cond or self.extend_visit:
                    changed_value = False
                    if self.extend_visit is False:
                        self.extend_visit = True
                        changed_value = True

                    if self.delete == True:
                        if n.args is not None:
                            for arg in n.args.exprs:
                                if isinstance(arg, ID):
                                    if arg.name == self.variables["phase"]:
                                        arg.name = "PHASE"
                                    elif arg.name == self.variables["round"]:
                                        arg.name = "ROUND"
                    else:
                        if n.args is not None:
                            for arg in n.args.exprs:
                                if isinstance(arg, ID):
                                    if arg.name == self.variables["phase"]:
                                        arg.name = "PHASE"

                    fref = self._parenthesize_unless_simple(n.name)
                    s = fref + '(' + self.visit(n.args) + ')'
                    if "send" in n.name.name:
                        self.send_reached = True
                    if changed_value:
                        self.extend_visit = False
                    return s
                elif self.mode == "update" or self.visit_cond:
                    changed_value = False
                    if self.extend_visit is False:
                        self.extend_visit = True
                        changed_value = True

                    if self.delete == True:
                        if n.args is not None:
                            for arg in n.args.exprs:
                                if isinstance(arg, ID):
                                    if arg.name == self.variables["phase"]:
                                        arg.name = "PHASE"
                                    elif arg.name == self.variables["round"]:
                                        arg.name = "ROUND"
                    else:
                        if n.args is not None:
                            for arg in n.args.exprs:
                                if isinstance(arg, ID):
                                    if arg.name == self.variables["phase"]:
                                        arg.name = "PHASE"

                    fref = self._parenthesize_unless_simple(n.name)
                    s = fref + '(' + self.visit(n.args) + ')'
                    ok = False
                    if self.send_reached:
                        ok = True
                    if "send" in n.name.name:
                        self.send_reached = True
                    if changed_value:
                        self.extend_visit = False
                    if ok:
                        return s
        else:
            if self.mode == "send" and not self.send_reached \
                    or self.visit_cond:
                fref = self._parenthesize_unless_simple(n.name)

                if self.delete == True:
                    if n.args is not None:
                        for arg in n.args.exprs:
                            if isinstance(arg, ID):
                                if arg.name == self.variables["phase"]:
                                    arg.name = "PHASE"
                                elif arg.name == self.variables["round"]:
                                    arg.name = "ROUND"
                else:
                    if n.args is not None:
                        for arg in n.args.exprs:
                            if isinstance(arg, ID):
                                if arg.name == self.variables["phase"]:
                                    arg.name = "PHASE"

                s = fref + '(' + self.visit(n.args) + ')'
                if "send" in n.name.name :
                    self.send_reached = True
                return s
            elif self.mode == "update" or self.visit_cond:
                fref = self._parenthesize_unless_simple(n.name)

                if self.delete == True:
                    if n.args is not None:
                        for arg in n.args.exprs:
                            if isinstance(arg, ID):
                                if arg.name == self.variables["phase"]:
                                    arg.name = "PHASE"
                                elif arg.name == self.variables["round"]:
                                    arg.name = "ROUND"
                else:
                    if n.args is not None:
                        for arg in n.args.exprs:
                            if isinstance(arg, ID):
                                if arg.name == self.variables["phase"]:
                                    arg.name = "PHASE"

                s = fref + '(' + self.visit(n.args) + ')'
                ok = False
                if self.send_reached:
                    ok = True
                if "send" in n.name.name:
                    self.send_reached = True
                if ok:
                    return s
        return ""

    def visit_UnaryOp(self, n):
        if self.path is not None:
            if n in self.path or self.extend_visit or self.visit_cond:
                if self.mode == "send" and not self.send_reached \
                        or self.mode == "update" and self.send_reached \
                        or self.visit_cond or self.extend_visit:
                    changed_value = False
                    if self.extend_visit is False:
                        self.extend_visit = True
                        changed_value = True
                    operand = self._parenthesize_unless_simple(n.expr)
                    if n.op == 'p++':
                        if changed_value:
                            self.extend_visit = False
                        return '%s++' % operand
                    elif n.op == 'p--':
                        if changed_value:
                            self.extend_visit = False
                        return '%s--' % operand
                    elif n.op == 'sizeof':
                        # Always parenthesize the argument of sizeof since it can be
                        # a name.
                        if changed_value:
                            self.extend_visit = False
                        return 'sizeof(%s)' % self.visit(n.expr)
                    else:
                        if changed_value:
                            self.extend_visit = False
                        return '%s%s' % (n.op, operand)
        else:
            if self.mode == "send" and not self.send_reached \
                    or self.mode == "update" and self.send_reached \
                    or self.visit_cond:
                operand = self._parenthesize_unless_simple(n.expr)
                if n.op == 'p++':
                    return '%s++' % operand
                elif n.op == 'p--':
                    return '%s--' % operand
                elif n.op == 'sizeof':
                    # Always parenthesize the argument of sizeof since it can be
                    # a name.
                    return 'sizeof(%s)' % self.visit(n.expr)
                else:
                    return '%s%s' % (n.op, operand)
        return ""

    def visit_BinaryOp(self, n):
        if self.path is not None:
            if n in self.path or self.extend_visit or self.visit_cond:
                if self.mode == "send" and not self.send_reached \
                        or self.mode == "update" and self.send_reached \
                        or self.visit_cond or self.extend_visit:
                    changed_value = False
                    if self.extend_visit is False:
                        self.extend_visit = True
                        changed_value = True
                    lval_str = self._parenthesize_if(n.left,
                                                     lambda d: not self._is_simple_node(d))
                    rval_str = self._parenthesize_if(n.right,
                                                     lambda d: not self._is_simple_node(d))
                    if changed_value:
                        self.extend_visit = False
                    return '%s %s %s' % (lval_str, n.op, rval_str)
        else:
            if self.mode == "send" and not self.send_reached \
                    or self.mode == "update" and self.send_reached \
                    or self.visit_cond:
                lval_str = self._parenthesize_if(n.left,
                                                 lambda d: not self._is_simple_node(d))
                rval_str = self._parenthesize_if(n.right,
                                                 lambda d: not self._is_simple_node(d))
                return '%s %s %s' % (lval_str, n.op, rval_str)
        return ""

    def visit_For(self, n):
        if self.path is not None:
            if n in self.path or self.extend_visit or self.visit_cond:
                if self.mode == "send" and not self.send_reached \
                        or self.mode == "update" and self.send_reached \
                        or self.visit_cond or self.extend_visit:
                    changed_value = False
                    if self.extend_visit is False:
                        self.extend_visit = True
                        changed_value = True
                    s = 'for ('
                    if n.init: s += self.visit(n.init)
                    s += ';'
                    if n.cond: s += ' ' + self.visit(n.cond)
                    s += ';'
                    if n.next: s += ' ' + self.visit(n.next)
                    s += ')\n'
                    s += self._generate_stmt(n.stmt, add_indent=True)
                    if changed_value:
                        self.extend_visit = False
                    return s
        else:
            if self.mode == "send" and not self.send_reached \
                    or self.mode == "update" and self.send_reached \
                    or self.visit_cond:
                s = 'for ('
                if n.init: s += self.visit(n.init)
                s += ';'
                if n.cond: s += ' ' + self.visit(n.cond)
                s += ';'
                if n.next: s += ' ' + self.visit(n.next)
                s += ')\n'
                s += self._generate_stmt(n.stmt, add_indent=True)
                return s

    def visit_Assignment(self, n):
        if self.path is not None:
            if n in self.path or self.extend_visit or self.visit_cond:
                if self.mode == "send" and not self.send_reached \
                        or self.mode == "update" and self.send_reached \
                        or self.visit_cond or self.extend_visit:
                    if self.mode == "update" and self.send_reached:
                        if n.lvalue.name == self.labelname and isinstance(n.rvalue, ID) \
                                and n.rvalue.name == "AUX_ROUND":
                            n.rvalue.name = "ERR_ROUND"

                    if self.delete:
                        if isinstance(n.rvalue, ID):
                            if n.rvalue.name == self.variables["round"]:
                                n.rvalue.name = "ROUND"
                            elif n.rvalue.name == self.variables["phase"]:
                                n.rvalue.name = "PHASE"
                    else:
                        if isinstance(n.rvalue, ID):
                            if n.rvalue.name == self.variables["phase"]:
                                n.rvalue.name = "PHASE"


                    changed_value = False
                    if self.extend_visit is False:
                        self.extend_visit = True
                        changed_value = True
                    rval_str = self._parenthesize_if(
                        n.rvalue,
                        lambda n: isinstance(n, c_ast.Assignment))
                    if changed_value:
                        self.extend_visit = False
                    return '%s %s %s' % (self.visit(n.lvalue), n.op, rval_str)
        else:
            if self.mode == "send" and not self.send_reached \
                    or self.mode == "update" and self.send_reached \
                    or self.visit_cond:
                if self.mode == "update" and self.send_reached:
                    if n.lvalue.name == self.labelname and isinstance(n.rvalue, ID) \
                            and n.rvalue.name == "AUX_ROUND":
                        # n.rvalue.name = self.first_round
                        return ""

                if self.delete:
                    if isinstance(n.rvalue, ID):
                        if n.rvalue.name == self.variables["round"]:
                            n.rvalue.name = "ROUND"
                        elif n.rvalue.name == self.variables["phase"]:
                            n.rvalue.name = "PHASE"
                else:
                    if isinstance(n.rvalue, ID):
                        if n.rvalue.name == self.variables["phase"]:
                            n.rvalue.name = "PHASE"

                rval_str = self._parenthesize_if(
                    n.rvalue,
                    lambda n: isinstance(n, c_ast.Assignment))
                return '%s %s %s' % (self.visit(n.lvalue), n.op, rval_str)
        return ""

    def visit_If(self, n):
        if (self.mode == "send" and not self.send_reached and self.path is None) \
                or (self.mode == "send" and not self.send_reached and self.path is not None
                    and n in self.path) or (self.mode == "send" and not self.send_reached and
                                            self.path is not None and self.extend_visit):
            s = ''
            if (self.path and n in self.path and n.iftrue and n.iftrue not in self.path and not n.iffalse) or \
                    (self.path and n in self.path and n.iftrue and n.iffalse
                     and n.iftrue not in self.path and n.iffalse not in self.path):
                if n.iftrue and not n.iffalse and len(n.iftrue.block_items) == 1:
                    child = n.iftrue.children()[0][1]
                    if isinstance(child, Assignment):
                        if isinstance(child.rvalue, ID):
                            if child.rvalue.name == self.current_round:
                                return ""

                if n.iffalse and not n.iftrue and len(n.iffalse.block_items) == 1:
                    child = n.iffalse.children()[0][1]
                    if isinstance(child, Assignment):
                        if isinstance(child.rvalue, ID):
                            if child.rvalue.name == self.current_round:
                                return ""
                self.extend_visit = True
                s = 'if ('
                self.visit_cond = True
                s += '('
                cond_visitor = CondVisitor(self.current_round, self.is_upon)
                s += cond_visitor.visit(n.cond)
                # s += self.visit(n.cond)
                s += ')'

                if cond_visitor.save_round != "":
                    self.remember_round = cond_visitor.save_round

                node = n
                if not node.iffalse or not node.iftrue:
                    while True:
                        new_node = None

                        if node.iftrue:
                            if len(node.iftrue.block_items) == 1:
                                if isinstance(node.iftrue.block_items[0], If):
                                    if_node = node.iftrue.block_items[0]
                                    if not if_node.iftrue or not if_node.iffalse:
                                        new_node = if_node

                        if node.iffalse:
                            if len(node.iffalse.block_items) == 1:
                                if isinstance(node.iffalse.block_items[0], If):
                                    if_node = node.iffalse.block_items[0]
                                    if not if_node.iftrue or not if_node.iffalse:
                                        new_node = if_node

                        if new_node is None:
                            break

                        node = new_node

                        s += ' && '
                        s += '('
                        s += cond_visitor.visit(node.cond)
                        s += ')'
                        if cond_visitor.save_round != "":
                            self.remember_round = cond_visitor.save_round

                # s += self.visit(n.cond)
                # TODO: Might need change here
                if self.current_round == self.remember_round:
                    if self.remember_round in s:
                        return ""

                n = node

                self.visit_cond = False
                s += ')\n'
                s_aux = self._generate_stmt(n.iftrue, add_indent=True)
                aux = copy.copy(s_aux)

                aux = aux.replace(" ", "")
                # print
                # print "aici{0}gata".format(aux)
                # print
                # print s_aux
                if aux == "{\n}\n":
                    return ""
                s += s_aux
                if n.iffalse:
                    s_aux = self._generate_stmt(n.iffalse, add_indent=True)
                    if (n.iftrue is not None) and (len(s_aux) > 0):
                        s += self._make_indent() + 'else\n'
                    aux = copy.copy(s_aux)
                    aux = aux.replace(" ", "")
                    # print s_aux
                    # print
                    # print "aici{0}gata".format(aux)
                    # print
                    if aux == "{\n}\n":
                        return ""
                    s += s_aux
                self.extend_visit = False
            elif self.path and n in self.path:
                if n.iftrue and not n.iffalse and len(n.iftrue.block_items) == 1:
                    child = n.iftrue.children()[0][1]
                    if isinstance(child, Assignment):
                        if isinstance(child.rvalue, ID):
                            if child.rvalue.name == self.current_round:
                                return ""

                if n.iffalse and not n.iftrue and len(n.iffalse.block_items) == 1:
                    child = n.iffalse.children()[0][1]
                    if isinstance(child, Assignment):
                        if isinstance(child.rvalue, ID):
                            if child.rvalue.name == self.current_round:
                                return ""
                s = 'if ('
                if n.cond:
                    if n.iffalse is not None and n.iffalse in self.path:
                        s += '!('
                    else:
                        s += '('
                    self.visit_cond = True
                    cond_visitor = CondVisitor(self.current_round, self.is_upon)
                    s += cond_visitor.visit(n.cond)
                    # s += self.visit(n.cond)

                    if cond_visitor.save_round != "":
                        self.remember_round = cond_visitor.save_round

                    self.visit_cond = False
                    if n.iffalse is not None and n.iffalse in self.path:
                        s += ')'
                    else:
                        s += ')'

                node = n
                if not node.iffalse or not node.iftrue:
                    while True:
                        new_node = None

                        if node.iftrue and node.iftrue in self.path:
                            if len(node.iftrue.block_items) == 1:
                                if isinstance(node.iftrue.block_items[0], If):
                                    if_node = node.iftrue.block_items[0]
                                    if not if_node.iftrue or not if_node.iffalse:
                                        new_node = if_node
                        elif node.iffalse:
                            if len(node.iffalse.block_items) == 1:
                                if isinstance(node.iffalse.block_items[0], If):
                                    if_node = node.iffalse.block_items[0]
                                    if not if_node.iftrue or not if_node.iffalse:
                                        new_node = if_node

                        if new_node is None:
                            break

                        node = new_node

                        s += ' && '
                        if node.iffalse is not None and node.iffalse in self.path:
                            s += '!('
                        else:
                            s += '('
                        cond_visitor = CondVisitor(self.current_round, self.is_upon)
                        s += cond_visitor.visit(node.cond)
                        s += ')'

                        if cond_visitor.save_round != "":
                            self.remember_round = cond_visitor.save_round

                # s += self.visit(n.cond)
                if self.current_round == self.remember_round:
                    if self.remember_round in s:
                        return ""

                n = node
                s += ')\n'

                if n.iftrue in self.path:
                    s_aux = self._generate_stmt(n.iftrue, add_indent=True)
                    aux = copy.copy(s_aux)
                    aux = aux.replace(" ", "")
                    # print
                    # print "aici{0}gata".format(aux)
                    # print
                    # print s_aux
                    if aux == "{\n}\n":
                        return ""
                    s += s_aux
                else:
                    if n.iffalse:
                        s_aux = self._generate_stmt(n.iffalse, add_indent=True)
                        aux = copy.copy(s_aux)
                        aux = aux.replace(" ", "")
                        # print "aici{0}gata".format(aux)
                        # print s_aux
                        if aux == "{\n}\n":
                            return ""
                        s += s_aux
            elif (self.path and self.extend_visit) or not self.path:
                if n.iftrue and not n.iffalse and len(n.iftrue.block_items) == 1:
                    child = n.iftrue.children()[0][1]
                    if isinstance(child, Assignment):
                        if isinstance(child.rvalue, ID):
                            if child.rvalue.name == self.current_round:
                                return ""

                if n.iffalse and not n.iftrue and len(n.iffalse.block_items) == 1:
                    child = n.iffalse.children()[0][1]
                    if isinstance(child, Assignment):
                        if isinstance(child.rvalue, ID):
                            if child.rvalue.name == self.current_round:
                                return ""
                if self.path and self.extend_visit:
                    s = 'if ('
                    self.visit_cond = True
                    s += '('
                    cond_visitor = CondVisitor(self.current_round, self.is_upon)
                    s += cond_visitor.visit(n.cond)
                    # s += self.visit(n.cond)
                    self.visit_cond = False


                    if cond_visitor.save_round != "":
                        self.remember_round = cond_visitor.save_round

                    s += ')'
                else:
                    s = 'if ('
                    if n.cond:
                        if n.iffalse is not None and n.iftrue is None:
                            s += '!('
                        else:
                            s += '('
                        cond_visitor = CondVisitor(self.current_round, self.is_upon)
                        s += cond_visitor.visit(n.cond)

                        if cond_visitor.save_round != "":
                            self.remember_round = cond_visitor.save_round

                        # s += self.visit(n.cond)
                        if n.iffalse is not None and n.iftrue is None:
                            s += ')'
                        else:
                            s += ')'

                node = n
                if not node.iffalse or not node.iftrue:
                    while True:
                        new_node = None

                        if node.iftrue:
                            if len(node.iftrue.block_items) == 1:
                                if isinstance(node.iftrue.block_items[0], If):
                                    if_node = node.iftrue.block_items[0]
                                    if not if_node.iftrue or not if_node.iffalse:
                                        new_node = if_node
                        if node.iffalse:
                            if len(node.iffalse.block_items) == 1:
                                if isinstance(node.iffalse.block_items[0], If):
                                    if_node = node.iffalse.block_items[0]
                                    if not if_node.iftrue or not if_node.iffalse:
                                        new_node = if_node

                        if new_node is None:
                            break

                        node = new_node

                        s += ' && '
                        if node.iffalse is not None and node.iftrue is None:
                            s += '!('
                        else:
                            s += '('
                        cond_visitor = CondVisitor(self.current_round, self.is_upon)
                        s += cond_visitor.visit(node.cond)

                        if cond_visitor.save_round != "":
                            self.remember_round = cond_visitor.save_round
                        s += ')'

                # s += self.visit(n.cond)
                if self.current_round == self.remember_round:
                    if self.remember_round in s:
                        return ""

                n = node

                s += ')\n'
                if n.iftrue:
                    s_aux = self._generate_stmt(n.iftrue, add_indent=True)
                    aux = copy.copy(s_aux)
                    aux = aux.replace(" ", "")
                    # print
                    # print "aici{0}gata".format(aux)
                    # print
                    # print s_aux
                    if aux == "{\n}\n":
                        return ""
                    s += s_aux
                if n.iffalse:
                    s_aux = self._generate_stmt(n.iffalse, add_indent=True)
                    if (n.iftrue is not None) and (len(s_aux) > 0):
                        s += self._make_indent() + 'else\n'

                    aux = copy.copy(s_aux)
                    aux = aux.replace(" ", "")
                    # print s_aux
                    # print
                    # print "aici{0}gata".format(aux)
                    # print
                    if aux == "{\n}\n":
                        return ""
                    s += s_aux
            return s

        elif (self.mode == "update" and self.path is None) or \
                (self.mode == "update" and self.path is not None and n in self.path) or \
                (self.mode == "update" and self.path is not None and self.extend_visit):
            ok1 = False
            ok2 = False
            s = ''

            if (self.path and n in self.path and n.iftrue and n.iftrue not in self.path and not n.iffalse) or \
                    (self.path and n in self.path and n.iftrue and n.iffalse
                     and n.iftrue not in self.path and n.iffalse not in self.path):
                if n.iftrue and not n.iffalse and len(n.iftrue.block_items) == 1:
                    child = n.iftrue.children()[0][1]
                    if isinstance(child, Assignment):
                        if isinstance(child.rvalue, ID):
                            if child.rvalue.name == self.current_round:
                                return ""

                if n.iffalse and not n.iftrue and len(n.iffalse.block_items) == 1:
                    child = n.iffalse.children()[0][1]
                    if isinstance(child, Assignment):
                        if isinstance(child.rvalue, ID):
                            if child.rvalue.name == self.current_round:
                                return ""

                self.extend_visit = True
                s = 'if ('
                self.visit_cond = True
                #### incepe
                s += '('
                cond_visitor = CondVisitor(self.current_round, self.is_upon)
                s += cond_visitor.visit(n.cond)
                s += ')'

                if cond_visitor.save_round != "":
                    self.remember_round = cond_visitor.save_round

                node = n
                if not node.iffalse or not node.iftrue:
                    while True:
                        new_node = None

                        if node.iftrue:
                            if len(node.iftrue.block_items) == 1:
                                if isinstance(node.iftrue.block_items[0], If):
                                    if_node = node.iftrue.block_items[0]
                                    if not if_node.iftrue or not if_node.iffalse:
                                        new_node = if_node

                        if node.iffalse:
                            if len(node.iffalse.block_items) == 1:
                                if isinstance(node.iffalse.block_items[0], If):
                                    if_node = node.iffalse.block_items[0]
                                    if not if_node.iftrue or not if_node.iffalse:
                                        new_node = if_node

                        if new_node is None:
                            break

                        node = new_node

                        s += ' && '
                        s += '('
                        s += cond_visitor.visit(node.cond)
                        s += ')'
                        if cond_visitor.save_round != "":
                            self.remember_round = cond_visitor.save_round


                # s += self.visit(n.cond)

                if self.current_round == self.remember_round:
                    if self.remember_round in s:
                        return ""

                n = node

                self.visit_cond = False
                s += ')\n'
                s_aux = self._generate_stmt(n.iftrue, add_indent=True)

                aux = copy.copy(s_aux)
                aux = aux.replace(" ", "")
                # print
                # print "aici{0}gata".format(aux)
                # print s_aux
                if aux == "{\n}\n":
                    return ""
                s += s_aux

                if self.send_last_instr:
                    ok1 = True
                    self.send_last_instr = False
                if n.iffalse:
                    if n.iftrue is not None:
                        s += self._make_indent() + 'else\n'
                    s += self._generate_stmt(n.iffalse, add_indent=True)
                    if self.send_last_instr:
                        ok2 = True
                        self.send_last_instr = False
                self.extend_visit = False
            elif self.path and n in self.path:
                if n.iftrue and not n.iffalse and len(n.iftrue.block_items) == 1:
                    child = n.iftrue.children()[0][1]
                    if isinstance(child, Assignment):
                        if isinstance(child.rvalue, ID):
                            if child.rvalue.name == self.current_round:
                                return ""

                if n.iffalse and not n.iftrue and len(n.iffalse.block_items) == 1:
                    child = n.iffalse.children()[0][1]
                    if isinstance(child, Assignment):
                        if isinstance(child.rvalue, ID):
                            if child.rvalue.name == self.current_round:
                                return ""
                s = 'if ('
                if n.cond:
                    if n.iffalse is not None and n.iffalse in self.path:
                        s += '!('
                    else:
                        s += '('
                    self.visit_cond = True
                    cond_visitor = CondVisitor(self.current_round, self.is_upon)
                    s += cond_visitor.visit(n.cond)

                    if cond_visitor.save_round != "":
                        self.remember_round = cond_visitor.save_round

                    # s += self.visit(n.cond)
                    self.visit_cond = False
                    if n.iffalse is not None and n.iffalse in self.path:
                        s += ')'
                    else:
                        s += ')'


                node = n
                if not node.iffalse or not node.iftrue:
                    while True:
                        new_node = None

                        if node.iftrue and node.iftrue in self.path:
                            if len(node.iftrue.block_items) == 1:
                                if isinstance(node.iftrue.block_items[0], If):
                                    if_node = node.iftrue.block_items[0]
                                    if not if_node.iftrue or not if_node.iffalse:
                                        new_node = if_node
                        elif node.iffalse:
                            if len(node.iffalse.block_items) == 1:
                                if isinstance(node.iffalse.block_items[0], If):
                                    if_node = node.iffalse.block_items[0]
                                    if not if_node.iftrue or not if_node.iffalse:
                                        new_node = if_node

                        if new_node is None:
                            break

                        node = new_node

                        s += ' && '
                        if node.iffalse is not None and node.iffalse in self.path:
                            s += '!('
                        else:
                            s += '('
                        cond_visitor = CondVisitor(self.current_round, self.is_upon)
                        s += cond_visitor.visit(node.cond)
                        s += ')'
                        if cond_visitor.save_round != "":
                            self.remember_round = cond_visitor.save_round

                # s += self.visit(n.cond)

                if self.current_round == self.remember_round:
                    if self.remember_round in s:
                        return ""

                n = node

                s += ')\n'
                if n.iftrue in self.path:
                    s_aux = self._generate_stmt(n.iftrue, add_indent=True)
                    aux = copy.copy(s_aux)
                    aux = aux.replace(" ", "")
                    # print
                    # print "aici{0}gata".format(aux)
                    # print
                    # print s_aux
                    if aux == "{\n}\n":
                        return ""
                    s += s_aux
                    if self.send_last_instr:
                        ok1 = True
                        self.send_last_instr = False
                else:
                    if n.iffalse:
                        s_aux = self._generate_stmt(n.iffalse, add_indent=True)
                        aux = copy.copy(s_aux)
                        aux = aux.replace(" ", "")
                        # print
                        # print "aici{0}gata".format(aux)
                        # print
                        # print s_aux
                        if aux == "{\n}\n":
                            return ""
                        s += s_aux
                        if self.send_last_instr:
                            ok2 = True
                            self.send_last_instr = False
            elif (self.path and self.extend_visit) or not self.path:
                if n.iftrue and not n.iffalse and len(n.iftrue.block_items) == 1:
                    child = n.iftrue.children()[0][1]
                    if isinstance(child, Assignment):
                        if isinstance(child.rvalue, ID):
                            if child.rvalue.name == self.current_round:
                                return ""

                if n.iffalse and not n.iftrue and len(n.iffalse.block_items) == 1:
                    child = n.iffalse.children()[0][1]
                    if isinstance(child, Assignment):
                        if isinstance(child.rvalue, ID):
                            if child.rvalue.name == self.current_round:
                                return ""
                if self.path and self.extend_visit:
                    s = 'if ('
                    s += '('
                    self.visit_cond = True
                    cond_visitor = CondVisitor(self.current_round, self.is_upon)
                    s += cond_visitor.visit(n.cond)

                    if cond_visitor.save_round != "":
                        self.remember_round = cond_visitor.save_round

                    # s += self.visit(n.cond)
                    self.visit_cond = False
                    s += ')'
                else:
                    s = 'if ('
                    if n.cond:
                        if n.iffalse is not None and n.iftrue is None:
                            s += '!('
                        else:
                            s += '('
                        self.visit_cond = True
                        cond_visitor = CondVisitor(self.current_round, self.is_upon)
                        s += cond_visitor.visit(n.cond)

                        if cond_visitor.save_round != "":
                            self.remember_round = cond_visitor.save_round


                        # s += self.visit(n.cond)
                        self.visit_cond = False
                        if n.iffalse is not None and n.iftrue is None:
                            s += ')'
                        else:
                            s += ')'

                node = n
                if not node.iffalse or not node.iftrue:
                    while True:
                        new_node = None

                        if node.iftrue:
                            if len(node.iftrue.block_items) == 1:
                                if isinstance(node.iftrue.block_items[0], If):
                                    if_node = node.iftrue.block_items[0]
                                    if not if_node.iftrue or not if_node.iffalse:
                                        new_node = if_node
                        if node.iffalse:
                            if len(node.iffalse.block_items) == 1:
                                if isinstance(node.iffalse.block_items[0], If):
                                    if_node = node.iffalse.block_items[0]
                                    if not if_node.iftrue or not if_node.iffalse:
                                        new_node = if_node

                        if new_node is None:
                            break

                        node = new_node

                        s += ' && '
                        if node.iffalse is not None and node.iftrue is None:
                            s += '!('
                        else:
                            s += '('
                        cond_visitor = CondVisitor(self.current_round, self.is_upon)
                        s += cond_visitor.visit(node.cond)
                        s += ')'

                        if cond_visitor.save_round != "":
                            self.remember_round = cond_visitor.save_round

                # s += self.visit(n.cond)
                # TODO: might need change in here
                if self.current_round == self.remember_round:
                    if self.remember_round in s:
                        return ""

                n = node

                s += ')\n'


                if n.iftrue:
                    s_aux = self._generate_stmt(n.iftrue, add_indent=True)
                    aux = copy.copy(s_aux)
                    aux = aux.replace(" ", "")
                    # print
                    # print "aici{0}gata".format(aux)
                    # print
                    # print s_aux
                    if aux == "{\n}\n":
                        return ""
                    s += s_aux
                    if self.send_last_instr:
                        ok1 = True
                        self.send_last_instr = False
                if n.iffalse:
                    s_aux = self._generate_stmt(n.iffalse, add_indent=True)
                    if n.iftrue is not None and len(s_aux) > 0:
                        s += self._make_indent() + 'else\n'
                    aux = copy.copy(s_aux)
                    aux = aux.replace(" ", "")

                    # print
                    # print "aici{0}gata".format(aux)
                    # print
                    # print s_aux
                    if aux == "{\n}\n":
                        return ""
                    s += s_aux
                    if self.send_last_instr:
                        ok2 = True
                        self.send_last_instr = False

            if self.send_reached:
                if not ok1 and not ok2:
                    return s
        return ""

    def visit_Compound(self, n):
        remeber_compound = False
        if self.first_compound:
            remeber_compound = True
            self.first_compound = False

        if (self.mode == "send" and not self.send_reached and self.path is None) \
                or (self.mode == "send" and not self.send_reached and self.path is not None
                    and n in self.path) or (self.mode == "send" and not self.send_reached and
                                            self.path is not None and self.extend_visit):
            s = ""
            out_break_reached = False
            if not remeber_compound:
                s = self._make_indent() + '{\n'
            self.indent_level += 2
            if n.block_items:
                for stmt in n.block_items:
                    if out_break_reached:
                        continue
                    if not self.send_reached:
                        if self.path is None or (self.path and stmt in self.path) or (self.path and self.extend_visit):
                            if self.mode == "send" and not self.send_reached:
                                if isinstance(stmt, Assignment):
                                    if stmt.lvalue.name == self.labelname:
                                        continue

                                    if self.delete:
                                        if stmt.op == "=" and isinstance(stmt.lvalue, StructRef) and \
                                                isinstance(stmt.lvalue.name, ID) and stmt.lvalue.name.name == self.message["name"] \
                                                and stmt.lvalue.type == "->" and isinstance(stmt.lvalue.field, ID) and \
                                                ((stmt.lvalue.field.name == self.message["phase_field"]) or (
                                                        stmt.lvalue.field.name == self.message["round_field"])):
                                            continue
                                    else:
                                        if stmt.op == "=" and isinstance(stmt.lvalue, StructRef) and \
                                                isinstance(stmt.lvalue.name, ID) and stmt.lvalue.name.name == self.message["name"] \
                                                and stmt.lvalue.type == "->" and isinstance(stmt.lvalue.field, ID) and \
                                                (stmt.lvalue.field.name == self.message["phase_field"]):
                                                    continue

                            if isinstance(stmt, Break):
                                out_break_reached = True
                            if isinstance(stmt, FuncCall):
                                if stmt.name.name == "out":
                                    out_break_reached = True
                            s += self._generate_stmt(stmt)
            self.indent_level -= 2
            if not remeber_compound:
                s += self._make_indent() + '}\n'

            # print "\n\n[[[[[[ {0} ]]]]]]\n\n".format(s)
            return s
        elif (self.mode == "update" and self.path is None) or \
                (self.mode == "update" and self.path is not None and n in self.path) or \
                (self.mode == "update" and self.path is not None and self.extend_visit):
            s = ""
            out_break_reached = False
            if not remeber_compound:
                s = self._make_indent() + '{\n'
            self.indent_level += 2
            if n.block_items:
                for index, stmt in enumerate(n.block_items):
                    if out_break_reached:
                        continue
                    if self.path is None or (self.path and stmt in self.path) or (self.path and self.extend_visit):
                        if not self.send_reached:
                            aux_s = self._generate_stmt(stmt)
                            if self.send_reached and isinstance(stmt, FuncCall):
                                if index == len(n.block_items) - 1:
                                    self.send_last_instr = True
                            elif self.send_reached:
                                s += aux_s
                        else:
                            if index < len(n.block_items) - 1:
                                if isinstance(stmt, Assignment):
                                    if isinstance(stmt.lvalue, ID) and "old" in stmt.lvalue.name:
                                        if isinstance(n.block_items[index + 1], If):
                                            aux = self._generate_stmt(n.block_items[index+1])
                                            aux = aux.replace(" ","")
                                            aux = aux.replace("\n","")
                                            if aux == "":
                                                continue


                            if isinstance(stmt, Assignment):
                                if isinstance(stmt.rvalue, ID) and stmt.rvalue.name == self.current_round:
                                    continue
                                if self.delete:
                                    if isinstance(stmt.lvalue, ID) and (stmt.lvalue.name == self.variables["round"] or stmt.lvalue.name == self.variables["phase"]):
                                            continue
                                else:
                                    if isinstance(stmt.lvalue, ID) and stmt.lvalue.name == self.variables["phase"]:
                                        continue

                            if isinstance(stmt, UnaryOp):
                                if stmt.op == "p++":
                                    if self.delete:
                                        if isinstance(stmt.expr, ID):
                                            if stmt.expr.name == self.variables["phase"] or stmt.expr.name == self.variables["round"]:
                                                continue
                                    else:
                                        if isinstance(stmt.expr, ID):
                                            if stmt.expr.name == self.variables["phase"]:
                                                continue

                            if isinstance(stmt, Break):
                                out_break_reached = True
                            if isinstance(stmt, FuncCall):
                                if stmt.name.name == "out":
                                    out_break_reached = True
                            s += self._generate_stmt(stmt)
            self.indent_level -= 2
            if not remeber_compound:
                s += self._make_indent() + '}\n'
            if self.send_reached and not self.send_last_instr:
                # print "\n\n[[[[[[ {0} ]]]]]]]\n\n".format(s)
                return s
        return ""


class CheckIfGenerator(c_generator.CGenerator):
    """
    This generator inspects the AST/a node and detects
    if it contains jumps of rounds or blocking loops.
    It takes as arguments the source node and the destination node.
    If the node doesn't contain any jumps or blocking loops,
    then the algorithm doesn't inspect it at all. This wouldn't be
    a good situation if inside the node is the source node or the
    destination node but no jumps or blocking loops because in this situation the node
    would remain unvisited and the source/dest node would never be
    reached.
    So self.is_jumping is True if the node must be visited (either there is a jump
    or there is only a source or a destination node). A true jump, i.e., another label
    assignment appears between the source and the destination is marked by self.true_jump.
    """

    def __init__(self, s, d):
        c_generator.CGenerator.__init__(self)
        self.is_jumping = False
        self.source = s
        self.dest = d
        self.label_name = s.lvalue.name
        self.is_blocking = False
        self.true_jump = False

    def visit_Assignment(self, n):
        if n.lvalue.name == self.label_name:
            self.is_jumping = True
            if not self.source == n and not self.dest == n:
                self.true_jump = True
        return ''

    def visit_FuncCall(self, n):
        if n.name.name == "wait_for_messages":
            self.is_blocking = True
        return ''
