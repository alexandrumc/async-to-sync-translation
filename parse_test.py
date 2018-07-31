from pycparser import parse_file, c_parser, c_generator, c_ast
from pycparser.c_ast import While, Assignment, ID, If, Node, FuncDef, FileAST, Constant, UnaryOp, Compound
from modify_whiles import whiles_to_if
import copy


def duplicate_element(element):
    return copy.deepcopy(element)


def get_extern_while_body(ast):
    if isinstance(ast, FileAST):
        for ext in ast.ext:
            if isinstance(ext, FuncDef) and ext.decl.name == "main":
                main_body = ext.body
                for operation in main_body:
                    if isinstance(operation, While):
                        return operation.stmt
    return ast


def get_extern_while(ast):
    if isinstance(ast, FileAST):
        for ext in ast.ext:
            if isinstance(ext, FuncDef) and ext.decl.name == "main":
                main_body = ext.body
                for operation in main_body:
                    if isinstance(operation, While):
                        return operation
    return ast


def get_main_function(ast):
    if isinstance(ast, FileAST):
        for ext in ast.ext:
            if isinstance(ext, FuncDef) and ext.decl.name == "main":
                return ext
    return ast


def prune_tree(current_node, lab_source, lab_dest, destination_reached, source_reached):
    to_delete = []
    for tupleChild in current_node.children():
        child = tupleChild[1]
        if not source_reached:
            if isinstance(child, If) is False:
                if child == lab_source:
                    source_reached.append(True)
                else:
                    to_delete.append(child)
                continue
            else:
                prune_tree(child.iftrue, lab_source, lab_dest, destination_reached, source_reached)
                if source_reached:
                    child.iffalse = None
                elif child.iffalse is not None:
                    child.iftrue = None
                    prune_tree(child.iffalse, lab_source, lab_dest, destination_reached, source_reached)
                    if not source_reached:
                        child.iffalse = None
                        to_delete.append(child)
                else:
                    to_delete.append(child)
                continue
        else:
            if not destination_reached:
                if isinstance(child, If) is False:
                    if child == lab_dest:
                        destination_reached.append(True)
                    continue
                else:
                    prune_tree(child.iftrue, lab_source, lab_dest, destination_reached, source_reached)
                    if destination_reached:
                        child.iffalse = None
                    elif child.iffalse is not None:
                        prune_tree(child.iffalse, lab_source, lab_dest, destination_reached, source_reached)
                        if destination_reached:
                            child.iftrue = None
                    continue
            else:
                to_delete.append(child)
    for node in to_delete:
        current_node.block_items.remove(node)


def prune_tree_mod(current_node, lab_source, lab_dest, destination_reached, source_reached,
                   source_ast, trees, parent_list, grandparent_list):
    to_delete = []
    if current_node is not None:
        grandparent = current_node

        for tupleChild in current_node.children():
            child = tupleChild[1]
            parent = child

            if not source_reached:
                if isinstance(child, If) is False:
                    if child == lab_source:
                        source_reached.append(True)
                    else:
                        to_delete.append(child)
                    continue
                else:
                    new_ast = duplicate_element(source_ast)

                    new_parent_list = []
                    for node in parent_list:
                        new_parent_list.append(find_node(new_ast, node))

                    new_grandparent_list = []
                    for node in grandparent_list:
                        new_grandparent_list.append(find_node(new_ast, node))

                    parent_list.append(parent)
                    grandparent_list.append(grandparent)

                    new_parent = find_node(new_ast, child)
                    new_grandparent = find_node(new_ast, current_node)

                    new_parent_list.append(new_parent)
                    new_grandparent_list.append(new_grandparent)

                    child.iffalse = None
                    new_parent.iftrue = None

                    prune_tree_mod(child.iftrue, lab_source, lab_dest, destination_reached, source_reached,
                                   source_ast, trees, parent_list, grandparent_list)
                    prune_tree_mod(new_parent.iffalse, lab_source, lab_dest, duplicate_element(destination_reached),
                                   duplicate_element(source_reached), new_ast, trees, new_parent_list,
                                   new_grandparent_list)

                    if source_reached:
                        child.iffalse = None
                    elif child.iffalse is not None:
                        child.iftrue = None
                        prune_tree_mod(child.iffalse, lab_source, lab_dest, destination_reached, source_reached,
                                       source_ast, trees, parent_list, grandparent_list)
                        if not source_reached:
                            child.iffalse = None
                            to_delete.append(child)
                    else:
                        to_delete.append(child)
                    continue
            else:
                if not destination_reached:
                    if isinstance(child, If) is False:
                        if child == lab_dest:
                            destination_reached.append(True)
                        continue
                    else:
                        prune_tree_mod(child.iftrue, lab_source, lab_dest, destination_reached, source_reached,
                                       source_ast, trees, parent_list, grandparent_list)
                        if destination_reached:
                            child.iffalse = None
                        elif child.iffalse is not None:
                            prune_tree_mod(child.iffalse, lab_source, lab_dest, destination_reached, source_reached,
                                           source_ast, trees, parent_list, grandparent_list)
                            if destination_reached:
                                child.iftrue = None
                        continue
                else:
                    to_delete.append(child)

    for node in to_delete:
        current_node.block_items.remove(node)

    if current_node == source_ast and source_reached and destination_reached:
        trees.append(current_node)


def find_all_paths_util(current_node, source_node, dest_node, path, parent_list, grandparent_list,
                        paths_list, source_reached):
    ok = True
    if current_node is not None:
        grandparent = current_node
        path.append(grandparent)
        ok = True
        for tupleChild in current_node.children():
            child = tupleChild[1]
            parent = child

            if isinstance(child, If):
                parent_list.append(parent)
                grandparent_list.append(grandparent)

                path1 = path[:]
                path2 = path[:]
                path1.append(parent)
                path2.append(parent)

                pl1 = parent_list[:]
                gp1 = grandparent_list[:]
                pl2 = parent_list[:]
                gp2 = grandparent_list[:]
                if child.iffalse is not None:
                    find_all_paths_util(child.iftrue, source_node, dest_node, path1, pl1, gp1, paths_list,
                                        source_reached)
                    find_all_paths_util(child.iffalse, source_node, dest_node, path2, pl2, gp2, paths_list,
                                        source_reached)
                else:
                    find_all_paths_util(child.iftrue, source_node, dest_node, path1, pl1, gp1, paths_list,
                                        source_reached)
                    find_all_paths_util(None, source_node, dest_node, path, pl2, gp2, paths_list,
                                        source_reached)
                ok = False
                break
            else:
                path.append(child)
                if child == source_node:
                    source_reached = True
                if child == dest_node:
                    if source_reached is True:
                        paths_list.append(path)
                    ok = False
                    break

    if parent_list and grandparent_list and ok is True:
        while grandparent_list:
            grandparent = grandparent_list[-1]
            parent = parent_list.pop()
            j = 0
            for j, tupleChild in enumerate(grandparent.children()):
                if tupleChild[1] == parent:
                    break
            remained_children = grandparent.children()[(j + 1):]
            for tupleChild in remained_children:
                child = tupleChild[1]
                parent = child
                if isinstance(child, If):
                    parent_list.append(parent)
                    path1 = path[:]
                    path2 = path[:]
                    path1.append(parent)
                    path2.append(parent)

                    pl1 = parent_list[:]
                    gp1 = grandparent_list[:]
                    pl2 = parent_list[:]
                    gp2 = grandparent_list[:]
                    if child.iffalse is not None:
                        find_all_paths_util(child.iftrue, source_node, dest_node, path1, pl1, gp1, paths_list,
                                            source_reached)
                        find_all_paths_util(child.iffalse, source_node, dest_node, path2, pl2, gp2, paths_list,
                                            source_reached)
                    else:
                        find_all_paths_util(child.iftrue, source_node, dest_node, path1, pl1, gp1, paths_list,
                                            source_reached)
                        find_all_paths_util(None, source_node, dest_node, path, pl2, gp2, paths_list, source_reached)
                    break
                else:
                    path.append(child)
                    if child == source_node:
                        source_reached = True
                    if child == dest_node:
                        if source_reached is True:
                            paths_list.append(path)
                        break
            grandparent_list.pop()


def find_all_paths_util_modified(current_node, source_node, dest_node, path, parent_list, grandparent_list,
                                 paths_list, source_reached, tree, last_if):
    to_delete = []
    ok = True
    if current_node is not None:
        grandparent = current_node
        path.append(grandparent)
        ok = True
        for tupleChild in current_node.children():
            child = tupleChild[1]
            parent = child

            if isinstance(child, If):
                parent_list.append(parent)
                grandparent_list.append(grandparent)

                path1 = path[:]
                path2 = path[:]
                path1.append(parent)
                path2.append(parent)

                pl1 = parent_list[:]
                gp1 = grandparent_list[:]
                pl2 = parent_list[:]
                gp2 = grandparent_list[:]
                if child.iffalse is not None:
                    find_all_paths_util_modified(child.iftrue, source_node, dest_node, path1, pl1, gp1, paths_list,
                                                 source_reached, tree, last_if)
                    find_all_paths_util_modified(child.iffalse, source_node, dest_node, path2, pl2, gp2, paths_list,
                                                 source_reached, tree, last_if)
                else:
                    find_all_paths_util_modified(child.iftrue, source_node, dest_node, path1, pl1, gp1, paths_list,
                                                 source_reached, tree, last_if)

                    new_tree = duplicate_element(tree)
                    new_child = find_node(new_tree, child)

                    new_child.cond = UnaryOp('!', child.cond, child.coord)
                    new_child.iffalse = Compound([], new_child.iftrue.coord)
                    new_child.iftrue = None

                    path.append(new_child)
                    path.append(new_child.iffalse)

                    new_parent_list = []
                    new_grandparent_list = []

                    for parent_node in parent_list:
                        new_parent_list.append(find_node(new_tree, parent_node))

                    for grandparent_node in grandparent_list:
                        new_grandparent_list.append(find_node(new_tree, grandparent_node))

                    find_all_paths_util_modified(None, source_node, dest_node, path, new_parent_list, new_grandparent_list, paths_list,
                                                 source_reached, new_tree, new_child.iffalse)

                ok = False
                break
            else:
                path.append(child)

                if last_if is not None:
                    last_if.block_items.append(child)
                    to_delete.append(child)

                if child == source_node:
                    source_reached = True

                if child == dest_node:
                    if source_reached is True:
                        paths_list.append((tree, path))
                    ok = False
                    break

        for node in to_delete:
            current_node.block_items.remove(node)

    if parent_list and grandparent_list and ok is True:
        while grandparent_list:
            to_delete = []
            grandparent = grandparent_list[-1]
            parent = parent_list.pop()
            j = 0
            for j, tupleChild in enumerate(grandparent.children()):
                if tupleChild[1] == parent:
                    break
            remained_children = grandparent.children()[(j + 1):]

            for tupleChild in remained_children:
                child = tupleChild[1]
                parent = child

                if isinstance(child, If):
                    parent_list.append(parent)

                    path1 = path[:]
                    path2 = path[:]
                    path1.append(parent)
                    path2.append(parent)

                    pl1 = parent_list[:]
                    gp1 = grandparent_list[:]
                    pl2 = parent_list[:]
                    gp2 = grandparent_list[:]

                    if child.iffalse is not None:
                        find_all_paths_util_modified(child.iftrue, source_node, dest_node, path1, pl1, gp1,
                                                     paths_list, source_reached, tree, last_if)
                        find_all_paths_util_modified(child.iffalse, source_node, dest_node, path2, pl2, gp2, paths_list,
                                                     source_reached, tree, last_if)
                    else:
                        find_all_paths_util_modified(child.iftrue, source_node, dest_node, path1, pl1, gp1, paths_list,
                                                     source_reached, tree, last_if)

                        new_tree = duplicate_element(tree)
                        new_child = find_node(new_tree, child)

                        new_child.cond = UnaryOp('!', child.cond, child.coord)
                        new_child.iffalse = Compound([], new_child.iftrue.coord)
                        new_child.iftrue = None

                        path.append(new_child)
                        path.append(new_child.iffalse)

                        new_parent_list = []
                        new_grandparent_list = []

                        for parent_node in parent_list:
                            new_parent_list.append(find_node(new_tree, parent_node))

                        for grandparent_node in grandparent_list:
                            new_grandparent_list.append(find_node(new_tree, grandparent_node))

                        find_all_paths_util_modified(None, source_node, dest_node, path, new_parent_list, new_grandparent_list, paths_list,
                                                     source_reached, new_tree, new_child.iffalse)
                    break
                else:
                    path.append(child)

                    if last_if is not None:
                        last_if.block_items.append(child)
                        to_delete.append(child)

                    if child == source_node:
                        source_reached = True

                    if child == dest_node:
                        if source_reached is True:
                            paths_list.append((tree, path))
                        break
            grandparent_list.pop()
            for node in to_delete:
                grandparent.block_items.remove(node)





def find_all_paths(root, source_node, dest_node):
    path = []
    parent_list = []
    grandparent_list = []
    paths_list = []
    find_all_paths_util(root, source_node, dest_node, path, parent_list, grandparent_list, paths_list, False)
    # print "\nDRUMURI GASITE:\n {0}".format(len(paths_list))
    return paths_list


def find_all_paths_modified(root, source_node, dest_node):
    path = []
    parent_list = []
    grandparent_list = []
    paths_list = []
    find_all_paths_util_modified(root, source_node, dest_node, path, parent_list, grandparent_list, paths_list,
                            False, root, None)
    #print "\nDRUMURI GASITE:\n {0}".format(len(paths_list))
    return paths_list


class LabelVisitor(c_ast.NodeVisitor):
    def __init__(self, label_name, label_value):
        self.label_name = label_name
        self.label_value = label_value
        self.label_item = []
        self.label_line = 0

    def visit_Assignment(self, node):
        if node.lvalue.name == self.label_name:
            if isinstance(node.rvalue, ID):
                if node.rvalue.name == self.label_value:
                    self.label_item.append(node)
                    self.label_line = node.coord.line
            elif isinstance(node.rvalue, Constant):
                if int(node.rvalue.value) == int(self.label_value):
                    self.label_item.append(node)
                    self.label_line = node.coord.line


class CheckLabelNumber(c_ast.NodeVisitor):
    def __init__(self, label_name):
        self.label_name = label_name
        self.count_labels = 0

    def visit_Assignment(self, node):
        if node.lvalue.name == self.label_name:
            self.count_labels += 1


class LocateChild(c_generator.CGenerator):
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


def find_node(ast_tree, node):
    v = LocateChild(node)
    v.visit(ast_tree)
    return v.discovered_node


def get_label_assign_num(ast_tree, label_name):
    v = CheckLabelNumber(label_name)
    v.visit(ast_tree)
    return v.count_labels


def get_label(ast_tree, label_name, label_value):
    v = LabelVisitor(label_name, label_value)
    v.visit(ast_tree)
    return v.label_item


def get_label_line(ast_tree, label_name, label_value):
    v = LabelVisitor(label_name, label_value)
    v.visit(ast_tree)
    return v.label_line


def find_all_paths_to_label(ast_tree, label_source, label_dest):
    extern_while_body = get_extern_while_body(ast_tree)
    return find_all_paths(extern_while_body, label_source, label_dest)


def find_all_paths_to_label_modified(ast_tree, label_source, label_dest):
    extern_while_body = get_extern_while_body(ast_tree)
    return find_all_paths_modified(extern_while_body, label_source, label_dest)


class PathGenerator(c_generator.CGenerator):
    def __init__(self, path):
        c_generator.CGenerator.__init__(self)
        self.path = path
        """
        Atunci cand se viziteaza o conditie mai complexa, ea e formata
        din mai multe elemente. In schimb, in path, conditia este stocata ca un singur
        element, iar visit_condition ii spune parser-ului ca poate sa afiseze elemente
        care nu sunt in path pentru ca viziteaza o conditie mai complexa care se afla
        in path
        """
        self.visit_condition = False

    def visit_Compound(self, n):
        if n in self.path:
            s = self._make_indent() + '{\n'
            self.indent_level += 2
            if n.block_items:
                s += ''.join(self._generate_stmt(stmt) for stmt in n.block_items if stmt in self.path)
            self.indent_level -= 2
            s += self._make_indent() + '}\n'
            return s
        else:
            return ''

    def visit_Assignment(self, n):
        if n in self.path:
            rval_str = self._parenthesize_if(
                n.rvalue,
                lambda n: isinstance(n, c_ast.Assignment))
            return '%s %s %s' % (self.visit(n.lvalue), n.op, rval_str)
        return ''

    def visit_If(self, n):
        if n in self.path:
            s = 'if ('
            self.visit_condition = True
            if n.cond:
                """
                if n.iffalse is None:
                    print "n.iffalse e None"
                else:
                    print "n.iffalse nu e None"

                if n.iftrue is None:
                    print "n.iftrue e None"
                else:
                    print "n.iftrue nu e None"

                if n.iffalse in self.path:
                    print "n.iffalse in path"
                else:
                    print "n.iffalse not in path"

                if n.iftrue in self.path:
                    print "n.iftrue in path"
                else:
                    print "n.iftrue not in path"
                """
                if n.iffalse is not None and n.iffalse in self.path \
                        and n.iftrue is not None:
                    s += '!'

                s += self.visit(n.cond)
            self.visit_condition = False
            s += ')\n'
            if n.iftrue in self.path:
                s += self._generate_stmt(n.iftrue, add_indent=True)
            else:
                if n.iffalse:
                    # s += self._make_indent()
                    s += self._generate_stmt(n.iffalse, add_indent=True)
            return s
        return ''

    def visit_While(self, n):
        s = 'int main ('
        s += ')\n'
        s += self._generate_stmt(n.stmt, add_indent=True)
        return s

    def visit_FuncCall(self, n):
        if n in self.path:
            fref = self._parenthesize_unless_simple(n.name)
            return fref + '(' + self.visit(n.args) + ')'
        return ''

    def visit_UnaryOp(self, n):
        if n in self.path or self.visit_condition:
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
        return ''

    def visit_BinaryOp(self, n):
        if n in self.path or self.visit_condition:
            lval_str = self._parenthesize_if(n.left,
                                             lambda d: not self._is_simple_node(d))
            rval_str = self._parenthesize_if(n.right,
                                             lambda d: not self._is_simple_node(d))
            return '%s %s %s' % (lval_str, n.op, rval_str)
        return ''


class LinesFinder(c_generator.CGenerator):
    def __init__(self, lower_bound, upper_bound):
        c_generator.CGenerator.__init__(self)
        self.upper_bound = upper_bound
        self.lower_bound = lower_bound

    def visit_If(self, n):
        s = ''
        if self.lower_bound <= n.coord.line <= self.upper_bound:
            s = 'if ('
            if n.cond: s += self.visit(n.cond)
            s += ')\n'
        s += self._generate_stmt(n.iftrue, add_indent=True)
        if n.iffalse:
            s += self._make_indent()
            if self.lower_bound <= n.iffalse.coord.line <= self.upper_bound:
                s += 'else\n'
            s += self._generate_stmt(n.iffalse, add_indent=True)
        return s

    def visit_Compound(self, n):
        s = self._make_indent()
        if self.lower_bound <= n.coord.line <= self.upper_bound:
            s += '{\n'
        self.indent_level += 2
        if n.block_items:
            s += ''.join(self._generate_stmt(stmt) for stmt in n.block_items)
        self.indent_level -= 2
        s += self._make_indent() + '}\n'
        return s

    def visit_While(self, n):
        s = ''
        if self.lower_bound <= n.coord.line <= self.upper_bound:
            s = 'while ('
            if n.cond: s += self.visit(n.cond)
            s += ')\n'
        s += self._generate_stmt(n.stmt, add_indent=True)
        return s

    def visit_UnaryOp(self, n):
        if self.lower_bound <= n.coord.line <= self.upper_bound:
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
        return ''

    def visit_BinaryOp(self, n):
        if self.lower_bound <= n.coord.line <= self.upper_bound:
            lval_str = self._parenthesize_if(n.left,
                                             lambda d: not self._is_simple_node(d))
            rval_str = self._parenthesize_if(n.right,
                                             lambda d: not self._is_simple_node(d))
            return '%s %s %s' % (lval_str, n.op, rval_str)
        return ''

    def visit_Assignment(self, n):
        if self.lower_bound <= n.coord.line <= self.upper_bound:
            rval_str = self._parenthesize_if(
                n.rvalue,
                lambda n: isinstance(n, c_ast.Assignment))
            return '%s %s %s' % (self.visit(n.lvalue), n.op, rval_str)
        return ''

    def visit_FuncCall(self, n):
        if self.lower_bound <= n.coord.line <= self.upper_bound:
            fref = self._parenthesize_unless_simple(n.name)
            return fref + '(' + self.visit(n.args) + ')'
        return ''

    def visit_Decl(self, n, no_type=False):
        # no_type is used when a Decl is part of a DeclList, where the type is
        # explicitly only for the first declaration in a list.
        #
        if self.lower_bound <= n.coord.line <= self.upper_bound:
            s = n.name if no_type else self._generate_decl(n)
            if n.bitsize: s += ' : ' + self.visit(n.bitsize)
            if n.init:
                s += ' = ' + self._visit_expr(n.init)
            return s
        return ''


def print_path(paths_list, index=-1):
    if index < 0:
        for path in paths_list:
            print "\n\n NEW PATH \n\n"
            for element in path:
                print "\n\nNEW ELEMENT\n\n"
                print element
    else:
        for element in paths_list[index]:
            print "\n\nNEW ELEMENT\n\n"
            print element


def generate_c_code_from_paths(paths_list, ast):
    for path in paths_list:
        gen = PathGenerator(path)
        print "\n\n\n\n NEW CODE \n\n\n\n"
        print(gen.visit(get_extern_while(ast)))

def generate_c_code_from_one_path(path, ast):
   gen = PathGenerator(path)
   print "\n\n\n\n NEW CODE \n\n\n\n"
   print(gen.visit(get_extern_while(ast)))




def generate_c_code_from_one_path(path, ast):
    gen = PathGenerator(path)
    print "\n\n\n\n NEW CODE \n\n\n\n"
    print(gen.visit(get_extern_while(ast)))


def generate_c_code_from_paths_and_trees(tuples):
    for pair in tuples:
        gen = PathGenerator(pair[1])
        print "\n\n\n\n NEW CODE \n\n\n\n"
        print(gen.visit(pair[0]))


def get_code_paths_list(paths_list, ast):
    result = []
    for path in paths_list:
        gen = PathGenerator(path)
        result.append(gen.visit(get_extern_while(ast)))
    return result


def find_lca(root, label1, label2):
    if root is None:
        return None

    if root is label1 or root is label2:
        return root

    count = 0
    save_child = None

    for pair in root.children():
        child = pair[1]
        return_val = find_lca(child, label1, label2)
        if return_val is not None:
            save_child = return_val
            count += 1
        if count == 2:
            return root

    return save_child


if __name__ == "__main__":
    # ast = parse_file(filename="/Users/alexandrum/ENS/pycparser/examples/c_files/two_phase.c", use_cpp=False)
    generator = c_generator.CGenerator()

    # whiles_to_if(get_extern_while_body(ast))

    # print generator.visit(ast)

    # print ast.show()
    """
    label1_list = get_label(ast, "lab", "1")
    label2_list = get_label(ast, "lab", "2")

    label_source = label1_list[0]
    for label_dest in label2_list:
        aux_ast = duplicate_element(ast)
        prune_tree(get_extern_while_body(aux_ast), label_source, label_dest, [], [])
        print generator.visit(get_extern_while_body(aux_ast))
        paths_list = find_all_paths_to_label(aux_ast, label_source, label_dest)
        generate_c_code_from_paths(paths_list, aux_ast)

    """
    """
    from take_lab import take_code_from_file
    code_paths = take_code_from_file(new_ast, "/Users/alexandrum/ENS/pycparser/examples/c_files/funky.c", "lab")
    print code_paths

    with open('somefile.c', 'w') as open_file:
        for frame in code_paths:
            for path in frame:
                open_file.write(path)
                open_file.write("\n")
    
    ast = parse_file(filename="/Users/alexandrum/ENS/pycparser/examples/c_files/two_phase.c", use_cpp=False)
    label1 = get_label(ast, "lab", "1")
    label2 = get_label(ast, "lab", "2")
    prune_tree(get_extern_while_body(ast), label1, label2, [], [])
    print(generator.visit(get_extern_while_body(ast)))
    paths_list = find_all_paths_to_label(ast, "lab", "1", "2")
    generate_c_code_from_paths(paths_list, ast)
    
    """
    ast = parse_file(filename="/Users/alexandrum/ENS/pycparser/examples/c_files/funky.c", use_cpp=False)
    #ast.show()

    label1_list = get_label(ast, "lab", "FIRST_ROUND")
    label2_list = get_label(ast, "lab", "SECOND_ROUND")
    #print label1_list
    #print label2_list

    #print ast

    #get_labels("/Users/alexandrum/ENS/pycparser/examples/c_files/funky.c", "lab")

    paths_list = []

    for source in label1_list:
        for dest in label2_list:
            aux_ast = duplicate_element(ast)
            whiles_to_if(get_extern_while_body(aux_ast))
            prune_tree(get_extern_while_body(aux_ast), source, dest, [], [])
            print generator.visit(get_extern_while_body(aux_ast))
            paths_list = find_all_paths_to_label_modified(aux_ast, source, dest)
            generate_c_code_from_paths_and_trees(paths_list)


