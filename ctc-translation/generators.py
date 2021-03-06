from pycparser.c_ast import While, Assignment, ID, If, Node, FuncDef, FileAST, Constant, UnaryOp, Compound, FuncCall, \
    Break, StructRef, BinaryOp
from pycparser import c_generator, c_ast
from modify_whiles import to_modify
import copy


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


class CondVisitor(c_generator.CGenerator):

    def __init__(self, current_round):
        c_generator.CGenerator.__init__(self)
        self.current_round = current_round
        self.inside_binop = False
        self.save_round = ""

    def visit_ID(self, n):
        if self.inside_binop is False:
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

        if isinstance(n.right, ID):
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


class SendVisitor(c_ast.NodeVisitor):
    """
    Returns a list with all occurences of send function
    """

    def __init__(self):
        self.list = []

    def visit_FuncCall(self, node):
        if node.name.name == 'send':
            self.list.append(node)


class RecvWhileVisitor(c_generator.CGenerator):
    """
    Identifies the recv whiles, used for asserts
    """

    def __init__(self):
        super(RecvWhileVisitor, self).__init__()
        self.list = []

    def visit_While(self, n):
        if to_modify(n):
            self.list.append(n)
        s = ""
        if n.cond:
            self.visit(n.cond)
        self._generate_stmt(n.stmt, add_indent=True)
        return s


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
        if self.node_to_find == n:
            self.discovered_node = n
        s = ''
        if n.iftrue:
            for stmt in n.iftrue.block_items:
                if stmt == self.node_to_find:
                    self.discovered_node = n
                    break
                self._generate_stmt(stmt)
        if n.iffalse:
            for stmt in n.iffalse.block_items:
                if stmt == self.node_to_find:
                    self.discovered_node = n
                    break
                self._generate_stmt(stmt)

        return s

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

    def __init__(self, mode, labelname, current_round, delete_round_phase, message, variables, first_round, save_round, path=None):
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
        self.first_round = first_round
        self.remember_round = save_round

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
                            n.rvalue.name = self.first_round

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
                        n.rvalue.name = self.first_round

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
                cond_visitor = CondVisitor(self.current_round)
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
                    if n.iftrue is not None:
                        s += self._make_indent() + 'else\n'
                    s_aux = self._generate_stmt(n.iffalse, add_indent=True)
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
                    cond_visitor = CondVisitor(self.current_round)
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
                        cond_visitor = CondVisitor(self.current_round)
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
                    cond_visitor = CondVisitor(self.current_round)
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
                        cond_visitor = CondVisitor(self.current_round)
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
                        cond_visitor = CondVisitor(self.current_round)
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
                    if n.iftrue is not None:
                        s += self._make_indent() + 'else\n'

                    s_aux = self._generate_stmt(n.iffalse, add_indent=True)
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
                cond_visitor = CondVisitor(self.current_round)
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
                    cond_visitor = CondVisitor(self.current_round)
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
                        cond_visitor = CondVisitor(self.current_round)
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
                    cond_visitor = CondVisitor(self.current_round)
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
                        cond_visitor = CondVisitor(self.current_round)
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
                        cond_visitor = CondVisitor(self.current_round)
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
                    if n.iftrue is not None:
                        s += self._make_indent() + 'else\n'
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
    If the node doesn't containt any jumps or blocking loops,
    then the algoritm doesn't inspect it at all. This wouldn't be
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
