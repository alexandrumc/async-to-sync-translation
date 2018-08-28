from pycparser import parse_file, c_parser, c_generator, c_ast
from pycparser.c_ast import While, Assignment, ID, If, Node, FuncDef, FileAST, Constant, UnaryOp, Compound, FuncCall
from modify_whiles import whiles_to_if
from modify_whiles import remove_mbox, identify_recv_exits
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


def find_all_paths_util_modified(current_node, source_node, dest_node, path, parent_list, grandparent_list, paths_list,
                                 source_reached, tree, last_if, parent_index, last_if_child):
    to_delete = []
    ok = True

    if current_node is not None:
        grandparent = current_node
        path.append(grandparent)
        ok = True
        for tupleChild in current_node.children():
            child = tupleChild[1]
            parent = child
            last_if_child_aux = True
            if isinstance(child, If):

                # nu pe parinti - avem nod if pe care facem recursivitate

                check_if_gen = CheckIfGenerator(source_node, dest_node)
                check_if_gen.visit(child.iftrue)
                jump_on_iftrue = check_if_gen.is_jumping
                jump_on_iffalse = False
                check_if_gen.is_jumping = False
                if child.iffalse is not None:
                    check_if_gen.visit(child.iffalse)
                    jump_on_iffalse = check_if_gen.is_jumping

                if not jump_on_iffalse and not jump_on_iftrue:
                    path.append(child)

                    if last_if is not None:
                        if find_node(last_if, child) is None:
                            last_if.block_items.append(child)
                            to_delete.append(child)
                    continue

                parent_list.append(parent)
                grandparent_list.append(grandparent)
                index = grandparent.block_items.index(parent)
                parent_index.append(index)

                other_children = current_node.block_items[(index + 1):]
                for other_child in other_children:
                    if isinstance(other_child, If):
                        last_if_child_aux = False

                path1 = path[:]
                path2 = path[:]
                path1.append(parent)
                path2.append(parent)

                pl1 = parent_list[:]
                gp1 = grandparent_list[:]
                pl2 = parent_list[:]
                gp2 = grandparent_list[:]

                pi1 = parent_index[:]
                pi2 = parent_index[:]

                if child.iffalse is not None:

                    # nu pe parinti - ambele ramuri ale lui if sunt True

                    new_tree_1 = duplicate_element(tree)
                    new_tree_2 = duplicate_element(tree)

                    new_parent_list_1 = []
                    new_grandparent_list_1 = []
                    new_grandparent_list_2 = []
                    new_parent_list_2 = []

                    for parent_node in parent_list:
                        new_parent_list_1.append(find_node(new_tree_1, parent_node))
                        new_parent_list_2.append(find_node(new_tree_2, parent_node))

                    for grandparent_node in grandparent_list:
                        new_grandparent_list_1.append(find_node(new_tree_1, grandparent_node))
                        new_grandparent_list_2.append(find_node(new_tree_2, grandparent_node))

                    if last_if is not None:

                        # nu pe parinti - ambele ramuri ale lui if sunt True si last_if este o ramura valida

                        last_if_in_new_tree_1 = find_node(new_tree_1, last_if)
                        if find_node(last_if_in_new_tree_1, child) is None:
                            last_if_in_new_tree_1.block_items.append(child)
                            to_delete.append(child)

                        last_if_in_new_tree_2 = find_node(new_tree_2, last_if)
                        if find_node(last_if_in_new_tree_2, child) is None:
                            last_if_in_new_tree_2.block_items.append(child)
                            if child not in to_delete:
                                to_delete.append(child)

                        new_grandparent_1 = find_node(new_tree_1, grandparent)
                        new_grandparent_2 = find_node(new_tree_2, grandparent)

                        for node in to_delete:
                            new_grandparent_1.block_items.remove(node)
                            new_grandparent_2.block_items.remove(node)

                        if len(to_delete) >= 2:
                            pi1[-1] = pi1[-1] - len(to_delete) + 1
                            pi2[-1] = pi2[-1] - len(to_delete) + 1

                        find_all_paths_util_modified(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                                     new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                                     last_if_in_new_tree_1, pi1, last_if_child_aux)

                        find_all_paths_util_modified(child.iffalse, source_node, dest_node, path2, new_parent_list_2,
                                                     new_grandparent_list_2, paths_list, source_reached, new_tree_2,
                                                     last_if_in_new_tree_2, pi2, last_if_child_aux)
                    else:

                        # nu pe parinti -ambele ramuri ale lui if sunt True si last_if e None

                        new_grandparent_1 = find_node(new_tree_1, grandparent)
                        new_grandparent_2 = find_node(new_tree_2, grandparent)

                        for node in to_delete:
                            new_grandparent_1.block_items.remove(node)
                            new_grandparent_2.block_items.remove(node)

                        if len(to_delete) >= 2:
                            pi1[-1] = pi1[-1] - len(to_delete) + 1
                            pi2[-1] = pi2[-1] - len(to_delete) + 1

                        find_all_paths_util_modified(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                                     new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                                     None, pi1, last_if_child_aux)

                        find_all_paths_util_modified(child.iffalse, source_node, dest_node, path2, new_parent_list_2,
                                                     new_grandparent_list_2, paths_list, source_reached, new_tree_2,
                                                     None, pi2, last_if_child_aux)
                else:

                    #  nu pe parinti - ramura iffalse nu exista

                    new_tree_1 = duplicate_element(tree)
                    new_parent_list_1 = []
                    new_grandparent_list_1 = []

                    for parent_node in parent_list:
                        new_parent_list_1.append(find_node(new_tree_1, parent_node))

                    for grandparent_node in grandparent_list:
                        new_grandparent_list_1.append(find_node(new_tree_1, grandparent_node))

                    if last_if is not None:

                        #  nu pe parinti - ramura iffalse nu exista si last_if e valida

                        last_if_in_new_tree_1 = find_node(new_tree_1, last_if)
                        if find_node(last_if_in_new_tree_1, child) is None:
                            last_if_in_new_tree_1.block_items.append(child)
                            to_delete.append(child)

                        new_grandparent_1 = find_node(new_tree_1, grandparent)

                        for node in to_delete:
                            new_grandparent_1.block_items.remove(node)

                        if len(to_delete) >= 2:
                            pi1[-1] = pi1[-1] - len(to_delete) + 1

                        find_all_paths_util_modified(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                                     new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                                     last_if_in_new_tree_1, pi1, last_if_child_aux)

                    else:

                        # nu pe parinti - ramura iffalse nu exista si last_if e None

                        new_grandparent_1 = find_node(new_tree_1, grandparent)

                        for node in to_delete:
                            new_grandparent_1.block_items.remove(node)

                        if len(to_delete) >= 2:
                            pi1[-1] = pi1[-1] - len(to_delete) + 1

                        find_all_paths_util_modified(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                                     new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                                     None, pi1, last_if_child_aux)

                    new_tree = duplicate_element(tree)
                    new_parent = find_node(new_tree, child)

                    gen = c_generator.CGenerator()
                    condition = ''
                    condition += gen.visit(child.cond)

                    new_parent.cond = ID(condition, child.coord)
                    new_parent.iffalse = Compound([], new_parent.iftrue.coord)
                    new_parent.iftrue = None

                    path.append(new_parent)
                    path.append(new_parent.iffalse)

                    new_parent_list = []
                    new_grandparent_list = []

                    for parent_node in parent_list:
                        new_parent_list.append(find_node(new_tree, parent_node))

                    for grandparent_node in grandparent_list:
                        new_grandparent_list.append(find_node(new_tree, grandparent_node))

                    if last_if is not None:
                        last_if_in_new_tree = find_node(new_tree, last_if)
                        if find_node(last_if_in_new_tree, new_parent) is None:
                            last_if_in_new_tree.block_items.append(new_parent)

                    new_grandparent = find_node(new_tree, grandparent)

                    for node in to_delete:
                        new_grandparent.block_items.remove(node)

                    if len(to_delete) >= 2:
                        pi2[-1] = pi2[-1] - len(to_delete) + 1

                    find_all_paths_util_modified(None, source_node, dest_node, path, new_parent_list,
                                                 new_grandparent_list, paths_list, source_reached, new_tree,
                                                 new_parent.iffalse, pi2, last_if_child_aux)

                ok = False
                break
            else:

                # nu pe parinti - avem nod normal

                path.append(child)

                if last_if is not None:
                    if find_node(last_if, child) is None:
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
            grandparent = grandparent_list.pop()
            parent = parent_list.pop()
            p_index = parent_index.pop()

            j = 0
            found_parent = False
            for j, tupleChild in enumerate(grandparent.children()):
                if tupleChild[1] == parent:
                    found_parent = True
                    break
            if found_parent:
                remained_children = grandparent.children()[(j + 1):]
            else:
                remained_children = grandparent.children()[p_index:]

            return_after_call = False

            for tupleChild in remained_children:
                child = tupleChild[1]
                parent = child

                last_if_child_aux = True
                if isinstance(child, If):

                    # ne intoarcem pe stramosi - avem nod if pe care facem recursivitate
                    check_if_gen = CheckIfGenerator(source_node, dest_node)
                    check_if_gen.visit(child.iftrue)
                    jump_on_iftrue = check_if_gen.is_jumping
                    jump_on_iffalse = False
                    check_if_gen.is_jumping = False
                    if child.iffalse is not None:
                        check_if_gen.visit(child.iffalse)
                        jump_on_iffalse = check_if_gen.is_jumping

                    if not jump_on_iffalse and not jump_on_iftrue:
                        path.append(child)

                        if last_if is not None:
                            if find_node(last_if, child) is None:
                                last_if.block_items.append(child)
                                to_delete.append(child)
                        continue

                    path1 = path[:]
                    path2 = path[:]
                    path1.append(parent)
                    path2.append(parent)

                    index = grandparent.block_items.index(parent)
                    other_children = grandparent.block_items[(index + 1):]
                    for other_child in other_children:
                        if isinstance(other_child, If):
                            last_if_child_aux = False

                    pl1 = parent_list[:]
                    pl1.append(parent)

                    gp1 = grandparent_list[:]
                    gp1.append(grandparent)

                    pl2 = parent_list[:]
                    pl2.append(parent)

                    gp2 = grandparent_list[:]
                    gp2.append(grandparent)

                    pi1 = parent_index[:]
                    pi1.append(grandparent.block_items.index(parent))

                    pi2 = parent_index[:]
                    pi2.append(grandparent.block_items.index(parent))

                    if child.iffalse is not None:

                        # ne intoarcem pe stramosi - ambele ramuri ale lui if sunt valide

                        new_tree_1 = duplicate_element(tree)
                        new_tree_2 = duplicate_element(tree)

                        new_parent_list_1 = []
                        new_grandparent_list_1 = []
                        new_grandparent_list_2 = []
                        new_parent_list_2 = []

                        for parent_node in pl1:
                            new_parent_list_1.append(find_node(new_tree_1, parent_node))
                            new_parent_list_2.append(find_node(new_tree_2, parent_node))

                        for grandparent_node in gp1:
                            new_grandparent_list_1.append(find_node(new_tree_1, grandparent_node))
                            new_grandparent_list_2.append(find_node(new_tree_2, grandparent_node))

                        if last_if is not None:

                            # ne intoarcem pe stramosi - ambele ramuri ale lui if sunt valide si last_if valid

                            last_if_in_new_tree_1 = find_node(new_tree_1, last_if)
                            if find_node(last_if_in_new_tree_1, child) is None:
                                last_if_in_new_tree_1.block_items.append(child)
                                to_delete.append(child)

                            last_if_in_new_tree_2 = find_node(new_tree_2, last_if)
                            if find_node(last_if_in_new_tree_2, child) is None:
                                last_if_in_new_tree_2.block_items.append(child)
                                if child not in to_delete:
                                    to_delete.append(child)

                            new_grandparent_1 = find_node(new_tree_1, grandparent)
                            new_grandparent_2 = find_node(new_tree_2, grandparent)

                            for node in to_delete:
                                new_grandparent_1.block_items.remove(node)
                                new_grandparent_2.block_items.remove(node)

                            if len(to_delete) >= 2:
                                pi1[-1] = pi1[-1] - len(to_delete) + 1
                                pi2[-1] = pi2[-1] - len(to_delete) + 1

                            find_all_paths_util_modified(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                                         new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                                         last_if_in_new_tree_1, pi1, last_if_child_aux)

                            find_all_paths_util_modified(child.iffalse, source_node, dest_node, path2,
                                                         new_parent_list_2, new_grandparent_list_2, paths_list,
                                                         source_reached, new_tree_2, last_if_in_new_tree_2, pi2,
                                                         last_if_child_aux)
                        else:

                            # ne intoarcem pe stramosi - ambele ramuri ale lui if sunt valide si last_if None

                            new_grandparent_1 = find_node(new_tree_1, grandparent)
                            new_grandparent_2 = find_node(new_tree_2, grandparent)

                            for node in to_delete:
                                new_grandparent_1.block_items.remove(node)
                                new_grandparent_2.block_items.remove(node)

                            if len(to_delete) >= 2:
                                pi1[-1] = pi1[-1] - len(to_delete) + 1
                                pi2[-1] = pi2[-1] - len(to_delete) + 1

                            find_all_paths_util_modified(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                                         new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                                         None, pi1, last_if_child_aux)

                            find_all_paths_util_modified(child.iffalse, source_node, dest_node, path2,
                                                         new_parent_list_2, new_grandparent_list_2, paths_list,
                                                         source_reached, new_tree_2, None, pi2,
                                                         last_if_child_aux)

                    else:

                        # ne intoarcem pe stramosi - ramura iffalse a lui if nu exista

                        new_tree_1 = duplicate_element(tree)
                        new_parent_list_1 = []
                        new_grandparent_list_1 = []

                        for parent_node in pl1:
                            new_parent_list_1.append(find_node(new_tree_1, parent_node))

                        for grandparent_node in gp1:
                            new_grandparent_list_1.append(find_node(new_tree_1, grandparent_node))

                        if last_if is not None:

                            # ne intoarcem pe stramosi - ramura iffalse a lui if nu exista si last_if valida

                            last_if_in_new_tree_1 = find_node(new_tree_1, last_if)
                            if find_node(last_if_in_new_tree_1, child) is None:
                                last_if_in_new_tree_1.block_items.append(child)
                                to_delete.append(child)

                            new_grandparent_1 = find_node(new_tree_1, grandparent)

                            for node in to_delete:
                                new_grandparent_1.block_items.remove(node)

                            if len(to_delete) >= 2:
                                pi1[-1] = pi1[-1] - len(to_delete) + 1

                            find_all_paths_util_modified(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                                         new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                                         last_if_in_new_tree_1, pi1, last_if_child_aux)

                        else:

                            # ne intoarcem pe stramosi - ramura iffalse a lui if nu exista si last_if e None

                            new_grandparent_1 = find_node(new_tree_1, grandparent)

                            for node in to_delete:
                                new_grandparent_1.block_items.remove(node)

                            if len(to_delete) >= 2:
                                pi1[-1] = pi1[-1] - len(to_delete) + 1

                            find_all_paths_util_modified(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                                         new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                                         None, pi1, last_if_child_aux)

                        new_tree = duplicate_element(tree)
                        new_parent = find_node(new_tree, child)

                        gen = c_generator.CGenerator()
                        condition = ''
                        condition += gen.visit(child.cond)

                        new_parent.cond = ID(condition, child.coord)
                        new_parent.iffalse = Compound([], new_parent.iftrue.coord)
                        new_parent.iftrue = None

                        path.append(new_parent)
                        path.append(new_parent.iffalse)

                        new_parent_list = []
                        new_grandparent_list = []

                        for parent_node in pl2:
                            new_parent_list.append(find_node(new_tree, parent_node))

                        for grandparent_node in gp2:
                            new_grandparent_list.append(find_node(new_tree, grandparent_node))

                        if last_if is not None:
                            last_if_in_new_tree = find_node(new_tree, last_if)
                            if find_node(last_if_in_new_tree, new_parent) is None:
                                last_if_in_new_tree.block_items.append(new_parent)

                        new_grandparent = find_node(new_tree, grandparent)

                        for node in to_delete:
                            new_grandparent.block_items.remove(node)

                        if len(to_delete) >= 2:
                            pi2[-1] = pi2[-1] - len(to_delete) + 1

                        find_all_paths_util_modified(None, source_node, dest_node, path, new_parent_list,
                                                     new_grandparent_list, paths_list, source_reached, new_tree,
                                                     new_parent.iffalse, pi2, last_if_child_aux)
                    return_after_call = True
                    break
                else:

                    # ne intoarcem pe stramosi - copil normal nu e if

                    path.append(child)

                    if last_if is not None:
                        if find_node(last_if, child) is None:
                            last_if.block_items.append(child)
                            to_delete.append(child)

                    if child == source_node:
                        source_reached = True

                    if child == dest_node:
                        if source_reached is True:
                            paths_list.append((tree, path))
                        break
            for node in to_delete:
                grandparent.block_items.remove(node)

            if last_if_child is False or return_after_call:
                break


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
    parent_index = []
    find_all_paths_util_modified(root, source_node, dest_node, path, parent_list, grandparent_list, paths_list, False,
                                 root, None, parent_index, True)
    # print "\nDRUMURI GASITE:\n {0}".format(len(paths_list))
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


class LocateParent(c_generator.CGenerator):
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


def find_node(ast_tree, node):
    v = LocateChild(node)
    v.visit(ast_tree)
    return v.discovered_node


def find_parent(ast_tree, child_node):
    v = LocateParent(child_node)
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


class TreeGenerator(c_generator.CGenerator):
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
        self.extend_visit = False

    def visit_Compound(self, n):
        if n in self.path or self.extend_visit:
            s = self._make_indent() + '{\n'
            self.indent_level += 2
            if n.block_items:
                s += ''.join(self._generate_stmt(stmt) for stmt in n.block_items if stmt in self.path or self.extend_visit)
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


class RoundGenerator(c_generator.CGenerator):
    def __init__(self, mode, path=None):
        c_generator.CGenerator.__init__(self)
        # send or update mode
        self.mode = mode
        # reached send operation, can begin to save operation for update phase
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

    """
    def visit_Constant(self, n):
        if self.mode == "send" and not self.send_reached \
                or self.mode == "update" and self.send_reached \
                or self.visit_cond:
            return n.value
        return ""

    def visit_ID(self, n):
        if self.mode == "send" and not self.send_reached \
                or self.mode == "update" and self.send_reached \
                or self.visit_cond:
            return n.name
        return ""

    def visit_Pragma(self, n):
        if self.mode == "send" and not self.send_reached \
                or self.mode == "update" and self.send_reached \
                or self.visit_cond:
            ret = '#pragma'
            if n.string:
                ret += ' ' + n.string
            return ret
        return ""

    def visit_ArrayRef(self, n):
        if self.mode == "send" and not self.send_reached \
                or self.mode == "update" and self.send_reached \
                or self.visit_cond:
            arrref = self._parenthesize_unless_simple(n.name)
            return arrref + '[' + self.visit(n.subscript) + ']'
        return ""

    def visit_StructRef(self, n):
        if self.mode == "send" and not self.send_reached \
                or self.mode == "update" and self.send_reached \
                or self.visit_cond:
            sref = self._parenthesize_unless_simple(n.name)
            return sref + n.type + self.visit(n.field)
        return ""
    """

    def visit_Decl(self, n, no_type=False):
        if self.path is not None:
            if n in self.path or self.extend_visit or self.visit_cond:
                if self.mode == "send" and not self.send_reached \
                        or self.mode == "update" and self.send_reached \
                        or self.visit_cond:
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
                        or self.visit_cond:
                    changed_value = False
                    if self.extend_visit is False:
                        self.extend_visit = True
                        changed_value = True
                    fref = self._parenthesize_unless_simple(n.name)
                    s = fref + '(' + self.visit(n.args) + ')'
                    if n.name.name == "send":
                        self.send_reached = True
                    if changed_value:
                        self.extend_visit = False
                    return s
                elif self.mode == "update" or self.visit_cond:
                    changed_value = False
                    if self.extend_visit is False:
                        self.extend_visit = True
                        changed_value = True
                    fref = self._parenthesize_unless_simple(n.name)
                    s = fref + '(' + self.visit(n.args) + ')'
                    ok = False
                    if self.send_reached:
                        ok = True
                    if n.name.name == "send":
                        self.send_reached = True
                    if changed_value:
                        self.extend_visit = False
                    if ok:
                        return s
        else:
            if self.mode == "send" and not self.send_reached \
                    or self.visit_cond:
                fref = self._parenthesize_unless_simple(n.name)
                s = fref + '(' + self.visit(n.args) + ')'
                if n.name.name == "send":
                    self.send_reached = True
                return s
            elif self.mode == "update" or self.visit_cond:
                fref = self._parenthesize_unless_simple(n.name)
                s = fref + '(' + self.visit(n.args) + ')'
                ok = False
                if self.send_reached:
                    ok = True
                if n.name.name == "send":
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
                    or self.visit_cond or self.extend_visit:
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
                        or self.visit_cond:
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

    def visit_Assignment(self, n):
        if self.path is not None:
            if n in self.path or self.extend_visit or self.visit_cond:
                if self.mode == "send" and not self.send_reached \
                        or self.mode == "update" and self.send_reached \
                        or self.visit_cond:
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
                self.extend_visit = True
                s = 'if ('
                self.visit_cond = True
                s += self.visit(n.cond)
                self.visit_cond = False
                s += ')\n'
                s += self._generate_stmt(n.iftrue, add_indent=True)
                if n.iffalse:
                    s += self._make_indent() + 'else\n'
                    s += self._generate_stmt(n.iffalse, add_indent=True)
                self.extend_visit = False
            elif self.path and n in self.path:
                s = 'if ('
                if n.cond:
                    if n.iffalse is not None and n.iffalse in self.path:
                        s += '!('
                    self.visit_cond = True
                    s += self.visit(n.cond)
                    self.visit_cond = False
                    if n.iffalse is not None and n.iffalse in self.path:
                        s += ')'
                s += ')\n'
                if n.iftrue in self.path:
                    s += self._generate_stmt(n.iftrue, add_indent=True)
                else:
                    if n.iffalse:
                        s += self._generate_stmt(n.iffalse, add_indent=True)
            elif (self.path and self.extend_visit) or not self.path:
                if self.path and self.extend_visit:
                    s = 'if ('
                    self.visit_cond = True
                    s += self.visit(n.cond)
                    self.visit_cond = False
                    s += ')\n'
                else:
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
                    s += self._make_indent() + 'else\n'
                    s += self._generate_stmt(n.iffalse, add_indent=True)
            return s

        elif (self.mode == "update" and self.path is None) or \
                (self.mode == "update" and self.path is not None and n in self.path):
            ok1 = False
            ok2 = False
            s = ''

            if (self.path and n in self.path and n.iftrue and n.iftrue not in self.path and not n.iffalse) or \
                (self.path and n in self.path and n.iftrue and n.iffalse
                 and n.iftrue not in self.path and n.iffalse not in self.path):
                self.extend_visit = True
                s = 'if ('
                self.visit_cond = True
                s += self.visit(n.cond)
                self.visit_cond = False
                s += ')\n'
                s += self._generate_stmt(n.iftrue, add_indent=True)
                if self.send_last_instr:
                    ok1 = True
                    self.send_last_instr = False
                if n.iffalse:
                    s += self._make_indent() + 'else\n'
                    s += self._generate_stmt(n.iffalse, add_indent=True)
                    if self.send_last_instr:
                        ok2 = True
                        self.send_last_instr = False
                self.extend_visit = False
            elif self.path and n in self.path:
                s = 'if ('
                if n.cond:
                    if n.iffalse is not None and n.iffalse in self.path:
                        s += '!('
                    self.visit_cond = True
                    s += self.visit(n.cond)
                    self.visit_cond = False
                    if n.iffalse is not None and n.iffalse in self.path:
                        s += ')'
                s += ')\n'
                if n.iftrue in self.path:
                    s += self._generate_stmt(n.iftrue, add_indent=True)
                    if self.send_last_instr:
                        ok1 = True
                        self.send_last_instr = False
                else:
                    if n.iffalse:
                        s += self._generate_stmt(n.iffalse, add_indent=True)
                        if self.send_last_instr:
                            ok2 = True
                            self.send_last_instr = False
            elif (self.path and self.extend_visit) or not self.path:
                if self.path and self.extend_visit:
                    s = 'if ('
                    self.visit_cond = True
                    s += self.visit(n.cond)
                    self.visit_cond = False
                    s += ')\n'
                else:
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
                    if self.send_last_instr:
                        ok1 = True
                        self.send_last_instr = False
                if n.iffalse:
                    s += self._make_indent() + 'else\n'
                    s += self._generate_stmt(n.iffalse, add_indent=True)
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
            if not remeber_compound:
                s = self._make_indent() + '{\n'
            self.indent_level += 2
            if n.block_items:
                for stmt in n.block_items:
                    if not self.send_reached:
                        if self.path is None or (self.path and stmt in self.path) or (self.path and self.extend_visit):
                            s += self._generate_stmt(stmt)
            self.indent_level -= 2
            if not remeber_compound:
                s += self._make_indent() + '}\n'
            return s
        elif (self.mode == "update" and self.path is None) or \
                (self.mode == "update" and self.path is not None and n in self.path) or\
                (self.mode == "update" and self.path is not None and self.extend_visit):
            s = ""
            if not remeber_compound:
                s = self._make_indent() + '{\n'
            self.indent_level += 2
            if n.block_items:
                for index, stmt in enumerate(n.block_items):
                    if self.path is None or (self.path and stmt in self.path) or (self.path and self.extend_visit):
                        if not self.send_reached:
                            aux_s = self._generate_stmt(stmt)
                            if self.send_reached and isinstance(stmt, FuncCall):
                                if index == len(n.block_items) - 1:
                                    self.send_last_instr = True
                            elif self.send_reached:
                                s += aux_s
                        else:
                            s += self._generate_stmt(stmt)
            self.indent_level -= 2
            if not remeber_compound:
                s += self._make_indent() + '}\n'
            if self.send_reached and not self.send_last_instr:
                return s
        return ""


class CheckIfGenerator(c_generator.CGenerator):
    def __init__(self, s, d):
        c_generator.CGenerator.__init__(self)
        self.is_jumping = False
        self.source = s
        self.dest = d
        self.label_name = s.lvalue.name

    def visit_Assignment(self, n):
        if n.lvalue.name == self.label_name:
            if n != self.source and n != self.dest:
                self.is_jumping = True
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

    # whiles_to_if(get_extern_while_body_from_func(ast))

    # print generator.visit(ast)

    # print ast.show()
    """
    label1_list = get_label(ast, "lab", "1")
    label2_list = get_label(ast, "lab", "2")

    label_source = label1_list[0]
    for label_dest in label2_list:
        aux_ast = duplicate_element(ast)
        prune_tree(get_extern_while_body_from_func(aux_ast), label_source, label_dest, [], [])
        print generator.visit(get_extern_while_body_from_func(aux_ast))
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
    prune_tree(get_extern_while_body_from_func(ast), label1, label2, [], [])
    print(generator.visit(get_extern_while_body_from_func(ast)))
    paths_list = find_all_paths_to_label(ast, "lab", "1", "2")
    generate_c_code_from_paths(paths_list, ast)
    """
    tree_gen = TreeGenerator()
    ast = parse_file(filename="examples/c_files/ct-terminating.c", use_cpp=False)
    cond = []
    whiles_to_if(get_extern_while_body(ast), cond)
    identify_recv_exits(get_extern_while_body(ast), cond)
    remove_mbox(get_extern_while_body(ast))

    # ast.show()
    #print tree_gen.visit(get_extern_while_body(ast))

    label1_list = get_label(ast, "round", "SECOND_ROUND")
    label2_list = get_label(ast, "round", "THIRD_ROUND")
    # print label1_list
    # print label2_list

    # print ast

    # get_labels("/Users/alexandrum/ENS/pycparser/examples/c_files/funky.c", "lab")

    paths_list = []

    #tree_gen = RoundGenerator("send")

    for source in label1_list:
        for dest in label2_list:
            aux_ast = duplicate_element(ast)
            dest_list = []
            source_list = []
            prune_tree(get_extern_while_body(aux_ast), source, dest, dest_list, source_list)
            if dest_list and source_list:
                #print tree_gen.visit(get_extern_while_body(aux_ast))
                pass
            # print "\n\nPAUZA\n\n"
            paths_list = find_all_paths_to_label_modified(aux_ast, source, dest)
            generate_c_code_from_paths_and_trees(paths_list)

# bug undeva cu 3 if-uri unul sub altul in exemplul ct-term
# intre second si third round
