from pycparser import c_generator, parse_file
from pycparser.c_ast import ID, If, Compound, FuncCall
from modify_whiles import whiles_to_if, identify_recv_exits
from generators import CheckIfGenerator, TreeGenerator
from utils import find_node, duplicate_element, get_label, get_extern_while_body, generate_c_code_from_paths_and_trees
from mbox_removal import remove_mbox


def prune_tree(current_node, lab_source, lab_dest, destination_reached, source_reached):
    """ The method keeps only the if conditions until it finds the source node.
    After reaching the source, it keeps every node between the source and the destination.
    Once the destination found, the method deletes every node that comes after the dest.

    :param current_node: program's extern while loop body in the first place, then changes at each recursive call
    :param lab_source:  source node; usually Assignment object, but it can be anything
    :param lab_dest:    destination node; usually Assignment object, but it can be anything
    :param destination_reached: empty list in the beginnig, append True when destination is found
    :param source_reached: empty list in the beginnig, append True when source is found
    :return: nothing. the pruning is done directly on the tree, i.e, the extern while loop body

    after the function was called, if there is at least one path between source and dest, the
    destination_reached and source_reached lists should not be empty
    """
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
            # First check if child is marker start/stop function
            # as this needs to be deleted, but only after source reached
            if not destination_reached:
                if isinstance(child, FuncCall) and (
                        child.name.name == "marker_start" or child.name.name == "marker_stop"):
                    to_delete.append(child)
                    continue
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


def find_all_paths_util(current_node, source_node, dest_node, path, parent_list, grandparent_list,
                        paths_list, source_reached, tree, last_if, parent_index, last_if_child, is_job):
    """

    :param current_node: program's extern while loop body just pruned with prune_tree function in the first place, then
    changes at each recursive call
    :param source_node: currently used with Assignment object, but it can be anything
    :param dest_node: currently used with Assignment object, but it can be anything
    :param path: a list where all the nodes along the path are stored
    :param parent_list: a list with all the parents that were visited
    :param grandparent_list: a list with all the grandparents that were visited
            If after visiting the children of a node we want to continue the search,
            we have to move to the next brother on the right side of that node. So we identify
            that brother by knowing the parent - the node that was just visited - and the grandparent, i.e,
            the parent of the if node
    :param paths_list: a list of tuples; a tuple is a pair of (tree, path), where the path is the path argument, i.e,
     a list of nodes and the tree is a clone of original tree modified to reflect the path. So each valid path
     discovered has its own tree.
    :param source_reached: boolean var that is set to True if the source was reached
    :param tree: the tree we are currently working with; for each modification, a clone is created, the nodes are
    moved/deleted/inserted by the case and the clone (now modified) is passed to the next recursive call
    :param last_if: a reference to an If node; the new discovered nodes have to be moved from their current place
     to become his children; if this argument is None, the discovered nodes remain in their places. it changes
     from None when an artificial else branch is created or there is a jump of rounds or a blocking while loop
    :param parent_index: sometimes the node of which the next right side brother is needed to continue the search
    is already deleted, so we have to keep in this list the node's old index so we can still detect his right
    side brother
    :param last_if_child: boolean var that is set according to the position of the current node as a child of his parent.
    if the current node is not the rightmost child of his parent, i.e, it has some brothers not yet visited in the right,
    we forbid this node to go back in the hierarchy more than 1 level to continue the search; if it would do that, the
    paths that will be find from those higher levels won't contain his right side brothers
    and this is not logical correct.
    :param is_job: is (j)ump (o)r (b)locking - if in the process of computing the paths the algorithm discovers a jump
    of rounds or a blocking while loop this argument is set to True and is used later in Round Generathing phase to
    wrap the rounds with the required if condition
    :return: nothing. at the end of the algorithm, the paths_list list given as an argument will be filled with tuples
    of (tree, path). is_job will also say if we need a wrapping
    """

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

                # exploring the tree - there's an if node and we recusively explore it

                check_if_gen = CheckIfGenerator(source_node, dest_node)
                check_if_gen.visit(child.iftrue)

                jump_on_iftrue = check_if_gen.is_jumping
                blocking_on_iftrue = check_if_gen.is_blocking

                jump_on_iffalse = False
                blocking_on_iffalse = False

                if check_if_gen.true_jump or check_if_gen.is_blocking:
                    if not is_job:
                        is_job.append(True)

                check_if_gen.true_jump = False
                check_if_gen.is_jumping = False
                check_if_gen.is_blocking = False

                if child.iffalse is not None:
                    check_if_gen.visit(child.iffalse)
                    jump_on_iffalse = check_if_gen.is_jumping
                    blocking_on_iffalse = check_if_gen.is_blocking

                if check_if_gen.true_jump or check_if_gen.is_blocking:
                    if not is_job:
                        is_job.append(True)

                if not jump_on_iffalse and not jump_on_iftrue \
                        and not blocking_on_iftrue and not blocking_on_iffalse:
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
                        break

                path1 = path[:]
                path2 = path[:]
                path1.append(parent)
                path2.append(parent)

                pi1 = parent_index[:]
                pi2 = parent_index[:]

                if jump_on_iftrue or blocking_on_iftrue:

                    if child.iffalse is not None:

                        # exploring the tree - jump on iftrue branch and there's an iffalse branch too

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

                            # exploring the three - jump on iftrue and there's an iffalse branch too
                            # last_if is valid

                            last_if_in_new_tree_1 = find_node(new_tree_1, last_if)
                            if find_node(last_if_in_new_tree_1, child) is None:
                                last_if_in_new_tree_1.block_items.append(child)
                                to_delete.append(child)

                            child_copy = duplicate_element(child)
                            last_if_in_new_tree_2 = find_node(new_tree_2, last_if)
                            if find_node(last_if_in_new_tree_2, child) is None:
                                last_if_in_new_tree_2.block_items.append(child_copy)
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

                            find_all_paths_util(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                                new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                                last_if_in_new_tree_1, pi1, last_if_child_aux, is_job)

                            find_all_paths_util(child.iffalse, source_node, dest_node, path2,
                                                new_parent_list_2, new_grandparent_list_2, paths_list,
                                                source_reached, new_tree_2, child_copy.iffalse, pi2,
                                                last_if_child_aux, is_job)

                        else:

                            # exploring the three - jump on iftrue and there's an iffalse branch too
                            # last_if is None

                            new_grandparent_1 = find_node(new_tree_1, grandparent)
                            new_grandparent_2 = find_node(new_tree_2, grandparent)

                            for node in to_delete:
                                new_grandparent_1.block_items.remove(node)
                                new_grandparent_2.block_items.remove(node)

                            if len(to_delete) >= 2:
                                pi1[-1] = pi1[-1] - len(to_delete) + 1
                                pi2[-1] = pi2[-1] - len(to_delete) + 1

                            new_last_if = find_node(new_tree_2, child.iffalse)

                            find_all_paths_util(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                                new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                                None, pi1, last_if_child_aux, is_job)

                            find_all_paths_util(child.iffalse, source_node, dest_node, path2,
                                                new_parent_list_2, new_grandparent_list_2, paths_list,
                                                source_reached, new_tree_2, new_last_if, pi2,
                                                last_if_child_aux, is_job)

                    else:
                        # exploring the three - jump on iftrue and there's no iffalse branch

                        new_tree_1 = duplicate_element(tree)
                        new_parent_list_1 = []
                        new_grandparent_list_1 = []

                        for parent_node in parent_list:
                            new_parent_list_1.append(find_node(new_tree_1, parent_node))

                        for grandparent_node in grandparent_list:
                            new_grandparent_list_1.append(find_node(new_tree_1, grandparent_node))

                        if last_if is not None:

                            # exploring the three - jump on iftrue and there's no iffalse branch
                            # last_if is valid

                            last_if_in_new_tree_1 = find_node(new_tree_1, last_if)
                            if find_node(last_if_in_new_tree_1, child) is None:
                                last_if_in_new_tree_1.block_items.append(child)
                                to_delete.append(child)

                            new_grandparent_1 = find_node(new_tree_1, grandparent)

                            for node in to_delete:
                                new_grandparent_1.block_items.remove(node)

                            if len(to_delete) >= 2:
                                pi1[-1] = pi1[-1] - len(to_delete) + 1

                            find_all_paths_util(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                                new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                                last_if_in_new_tree_1, pi1, last_if_child_aux, is_job)

                        else:

                            # exploring the three - jump on iftrue there's no iffalse branch
                            # last_if is None

                            new_grandparent_1 = find_node(new_tree_1, grandparent)

                            for node in to_delete:
                                new_grandparent_1.block_items.remove(node)

                            if len(to_delete) >= 2:
                                pi1[-1] = pi1[-1] - len(to_delete) + 1

                            find_all_paths_util(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                                new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                                None, pi1, last_if_child_aux, is_job)

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

                        find_all_paths_util(None, source_node, dest_node, path, new_parent_list,
                                            new_grandparent_list, paths_list, source_reached, new_tree,
                                            new_parent.iffalse, pi2, last_if_child_aux, is_job)

                elif jump_on_iffalse or blocking_on_iffalse:

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
                        # exploring the tree - jump on iffalse branch
                        # last_if is valid

                        child_copy = duplicate_element(child)
                        last_if_in_new_tree_1 = find_node(new_tree_1, last_if)
                        if find_node(last_if_in_new_tree_1, child) is None:
                            last_if_in_new_tree_1.block_items.append(child_copy)
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

                        find_all_paths_util(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                            new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                            child_copy.iftrue, pi1, last_if_child_aux, is_job)

                        find_all_paths_util(child.iffalse, source_node, dest_node, path2, new_parent_list_2,
                                            new_grandparent_list_2, paths_list, source_reached, new_tree_2,
                                            last_if_in_new_tree_2, pi2, last_if_child_aux, is_job)

                    else:
                        # exploring the tree - jump on iffalse branch
                        # last_if is None

                        new_grandparent_1 = find_node(new_tree_1, grandparent)
                        new_grandparent_2 = find_node(new_tree_2, grandparent)

                        for node in to_delete:
                            new_grandparent_1.block_items.remove(node)
                            new_grandparent_2.block_items.remove(node)

                        if len(to_delete) >= 2:
                            pi1[-1] = pi1[-1] - len(to_delete) + 1
                            pi2[-1] = pi2[-1] - len(to_delete) + 1

                        new_last_if = find_node(new_tree_1, child.iftrue)

                        find_all_paths_util(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                            new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                            new_last_if, pi1, last_if_child_aux, is_job)

                        find_all_paths_util(child.iffalse, source_node, dest_node, path2, new_parent_list_2,
                                            new_grandparent_list_2, paths_list, source_reached, new_tree_2,
                                            None, pi2, last_if_child_aux, is_job)

                ok = False
                break
            else:

                # exploring the tree - normal node != if node

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

    # back on the ancestors to continue the tree exploration
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

                    # back on the ancestors - if node left unvisited

                    check_if_gen = CheckIfGenerator(source_node, dest_node)
                    check_if_gen.visit(child.iftrue)

                    jump_on_iftrue = check_if_gen.is_jumping
                    blocking_on_iftrue = check_if_gen.is_blocking

                    if check_if_gen.true_jump or check_if_gen.is_blocking:
                        if not is_job:
                            is_job.append(True)

                    jump_on_iffalse = False
                    blocking_on_iffalse = False

                    check_if_gen.is_jumping = False
                    check_if_gen.is_blocking = False
                    check_if_gen.true_jump = False

                    if child.iffalse is not None:
                        check_if_gen.visit(child.iffalse)
                        jump_on_iffalse = check_if_gen.is_jumping
                        blocking_on_iffalse = check_if_gen.is_blocking

                    if check_if_gen.true_jump or check_if_gen.is_blocking:
                        if not is_job:
                            is_job.append(True)

                    if not jump_on_iffalse and not jump_on_iftrue \
                            and not blocking_on_iftrue and not blocking_on_iffalse:
                        path.append(child)

                        if last_if is not None:
                            if find_node(last_if, child) is None:
                                last_if.block_items.append(child)
                                to_delete.append(child)

                        if remained_children.index(tupleChild) == len(remained_children) - 1:
                            last_if_child = True

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
                            break

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

                    if jump_on_iftrue or blocking_on_iftrue:

                        if child.iffalse is not None:

                            # exploring the tree - jump on iftrue and there's an iffalse branch

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

                                # exploring the tree - jump on iftrue and there's an iffalse branch
                                # last_if is valid

                                last_if_in_new_tree_1 = find_node(new_tree_1, last_if)
                                if find_node(last_if_in_new_tree_1, child) is None:
                                    last_if_in_new_tree_1.block_items.append(child)
                                    to_delete.append(child)

                                child_copy = duplicate_element(child)
                                last_if_in_new_tree_2 = find_node(new_tree_2, last_if)
                                if find_node(last_if_in_new_tree_2, child) is None:
                                    last_if_in_new_tree_2.block_items.append(child_copy)
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

                                find_all_paths_util(child.iftrue, source_node, dest_node, path1,
                                                    new_parent_list_1, new_grandparent_list_1, paths_list,
                                                    source_reached, new_tree_1, last_if_in_new_tree_1, pi1,
                                                    last_if_child_aux, is_job)

                                find_all_paths_util(child.iffalse, source_node, dest_node, path2,
                                                    new_parent_list_2, new_grandparent_list_2, paths_list,
                                                    source_reached, new_tree_2, child_copy.iffalse, pi2,
                                                    last_if_child_aux, is_job)

                            else:

                                # exploring the tree - jump on iftrue and there's an iffalse branch
                                # last_if is None

                                new_grandparent_1 = find_node(new_tree_1, grandparent)
                                new_grandparent_2 = find_node(new_tree_2, grandparent)

                                for node in to_delete:
                                    new_grandparent_1.block_items.remove(node)
                                    new_grandparent_2.block_items.remove(node)

                                if len(to_delete) >= 2:
                                    pi1[-1] = pi1[-1] - len(to_delete) + 1
                                    pi2[-1] = pi2[-1] - len(to_delete) + 1

                                new_last_if = find_node(new_tree_2, child.iffalse)

                                find_all_paths_util(child.iftrue, source_node, dest_node, path1,
                                                    new_parent_list_1, new_grandparent_list_1, paths_list,
                                                    source_reached, new_tree_1, None, pi1, last_if_child_aux,
                                                    is_job)

                                find_all_paths_util(child.iffalse, source_node, dest_node, path2,
                                                    new_parent_list_2, new_grandparent_list_2, paths_list,
                                                    source_reached, new_tree_2, new_last_if, pi2,
                                                    last_if_child_aux, is_job)

                        else:
                            # exploring the tree - jump on iftrue and there's no iffalse branch

                            new_tree_1 = duplicate_element(tree)
                            new_parent_list_1 = []
                            new_grandparent_list_1 = []

                            for parent_node in pl1:
                                new_parent_list_1.append(find_node(new_tree_1, parent_node))

                            for grandparent_node in gp1:
                                new_grandparent_list_1.append(find_node(new_tree_1, grandparent_node))

                            if last_if is not None:

                                # exploring the tree - jump on iftrue and there's no iffalse branch
                                # last_if is valid

                                last_if_in_new_tree_1 = find_node(new_tree_1, last_if)
                                if find_node(last_if_in_new_tree_1, child) is None:
                                    last_if_in_new_tree_1.block_items.append(child)
                                    to_delete.append(child)

                                new_grandparent_1 = find_node(new_tree_1, grandparent)

                                for node in to_delete:
                                    new_grandparent_1.block_items.remove(node)

                                if len(to_delete) >= 2:
                                    pi1[-1] = pi1[-1] - len(to_delete) + 1

                                find_all_paths_util(child.iftrue, source_node, dest_node, path1,
                                                    new_parent_list_1, new_grandparent_list_1, paths_list,
                                                    source_reached, new_tree_1, last_if_in_new_tree_1, pi1,
                                                    last_if_child_aux, is_job)

                            else:

                                # exploring the tree - jump on iftrue and there's no iffalse branch
                                # last_if is None

                                new_grandparent_1 = find_node(new_tree_1, grandparent)

                                for node in to_delete:
                                    new_grandparent_1.block_items.remove(node)

                                if len(to_delete) >= 2:
                                    pi1[-1] = pi1[-1] - len(to_delete) + 1

                                find_all_paths_util(child.iftrue, source_node, dest_node, path1,
                                                    new_parent_list_1, new_grandparent_list_1, paths_list,
                                                    source_reached, new_tree_1, None, pi1, last_if_child_aux, is_job)

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

                            find_all_paths_util(None, source_node, dest_node, path, new_parent_list,
                                                new_grandparent_list, paths_list, source_reached, new_tree,
                                                new_parent.iffalse, pi2, last_if_child_aux, is_job)

                    elif jump_on_iffalse or blocking_on_iffalse:
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
                            # exploring the tree - jump on iffalse
                            # last_if is valid

                            child_copy = duplicate_element(child)
                            last_if_in_new_tree_1 = find_node(new_tree_1, last_if)
                            if find_node(last_if_in_new_tree_1, child) is None:
                                last_if_in_new_tree_1.block_items.append(child_copy)
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

                            find_all_paths_util(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                                new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                                child_copy.iftrue, pi1, last_if_child_aux, is_job)

                            find_all_paths_util(child.iffalse, source_node, dest_node, path2,
                                                new_parent_list_2, new_grandparent_list_2, paths_list,
                                                source_reached, new_tree_2, last_if_in_new_tree_2, pi2,
                                                last_if_child_aux, is_job)

                        else:
                            # exploring the tree - jump on iffalse
                            # last_if is None

                            new_grandparent_1 = find_node(new_tree_1, grandparent)
                            new_grandparent_2 = find_node(new_tree_2, grandparent)

                            for node in to_delete:
                                new_grandparent_1.block_items.remove(node)
                                new_grandparent_2.block_items.remove(node)

                            if len(to_delete) >= 2:
                                pi1[-1] = pi1[-1] - len(to_delete) + 1
                                pi2[-1] = pi2[-1] - len(to_delete) + 1

                            new_last_if = find_node(new_tree_1, child.iftrue)

                            find_all_paths_util(child.iftrue, source_node, dest_node, path1, new_parent_list_1,
                                                new_grandparent_list_1, paths_list, source_reached, new_tree_1,
                                                new_last_if, pi1, last_if_child_aux, is_job)

                            find_all_paths_util(child.iffalse, source_node, dest_node, path2,
                                                new_parent_list_2, new_grandparent_list_2, paths_list,
                                                source_reached, new_tree_2, None, pi2, last_if_child_aux, is_job)

                    return_after_call = True
                    break
                else:

                    # back on the ancestors - normal children are handled on this case, not if nodes

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


def find_all_paths_between_two_nodes(ast_tree, source_node, dest_node):
    extern_while_body = get_extern_while_body(ast_tree)
    path = []
    parent_list = []
    grandparent_list = []
    paths_list = []
    parent_index = []
    is_job = []
    find_all_paths_util(extern_while_body, source_node, dest_node, path, parent_list, grandparent_list, paths_list, False,
                        extern_while_body, None, parent_index, True, is_job)
    return paths_list, is_job


if __name__ == "__main__":
    generator = c_generator.CGenerator()
    tree_gen = TreeGenerator()
    ast = parse_file(filename="examples/c_files/tpc_AMIT_modificat.c", use_cpp=False)
    cond = []
    whiles_to_if(get_extern_while_body(ast), cond)
    identify_recv_exits(get_extern_while_body(ast), cond)
    remove_mbox(get_extern_while_body(ast))

    # ast.show()
    # print tree_gen.visit(get_extern_while_body(ast))

    label1_list = get_label(ast, "round", "FIRST_ROUND")
    label2_list = get_label(ast, "round", "AUX_ROUND")

    for source in label1_list:
        for dest in label2_list:
            aux_ast = duplicate_element(ast)
            dest_list = []
            source_list = []
            prune_tree(get_extern_while_body(aux_ast), source, dest, dest_list, source_list)
            if dest_list and source_list:
                # print tree_gen.visit(get_extern_while_body(aux_ast))
                path_list, is_job_detected = find_all_paths_between_two_nodes(aux_ast, source, dest)
                generate_c_code_from_paths_and_trees(path_list)