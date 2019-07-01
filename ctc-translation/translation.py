from take_lab import take_code_from_file, get_extern_while_body_from_func, get_paths_trees, turn_nested_algo_marked_compound, print_rounds
from pycparser import parse_file
from utils import duplicate_element, find_parent

from modify_whiles import *
import sys
import config
from mbox_removal import remove_mbox

ast = parse_file(filename=sys.argv[1], use_cpp=False)
x = get_extern_while_body_from_func(ast, "main")

# If we have only one algorithm then process it directly
if config.number_of_nested_algorithms == 1:
    conditions = []
    whiles_to_if(x, conditions)

    identify_recv_exits(x, conditions)

    mess_names = []
    for d in config.msg_structure_fields:
        mess_names.append(d['name'])

    for d in config.mailbox:
        mess_names.append(d)

    # Take dispose functions from config file
    func_names = []
    for d in config.clean_mailbox:
        func_names.append(d)
    remove_mbox(x, mess_names, func_names)

    take_code_from_file(ast, sys.argv[1], config.variables[0]['round'],
                        config.rounds_list[0], config.delete_round_phase[0], config.msg_structure_fields[0],
                        config.variables[0])
else:
    # Turn all while algos into if (cond) loop_body

    nested_algos_details = []

    turn_nested_algo_marked_compound(x, nested_algos_details, config.rounds_list, config.msg_structure_fields)

    conditions = []
    whiles_to_if(x, conditions)

    identify_recv_exits(x, conditions)

    # Delete unnecessary operations, like disposes and timeouts from code
    # First, get a list of all messages names

    mess_names = []
    for d in config.msg_structure_fields:
        mess_names.append(d['name'])

    for d in config.mailbox:
        mess_names.append(d)

    # Take dispose functions from config file
    func_names = []
    for d in config.clean_mailbox:
        func_names.append(d)
    remove_mbox(x, mess_names, func_names)

    # Start printing output
    print "Launched procedure for nested algorithms\n\n"

    # Iterate through algorithms and print them
    i = len(config.rounds_list)
    i -= 1

    while i >= 0:
        letter = str(unichr(65 + i))
        print "Algorithm " + letter + "\n\n"

        labs = duplicate_element(config.rounds_list[i])
        labs.append('ERR_ROUND')

        # Find this algo in code, delete markers
        # and update pointers for the whole deletion
        #del_reminder = []
        #for det in nested_algos_details:
        #    if det[0] == i:
        #        start_pos = det[3].block_items.index(det[1])
        #        fin_pos = det[3].block_items.index(det[2])

        #        print det[3].block_items[fin_pos-1].rvalue.name

        #        del_reminder.append((det[3].block_items[start_pos + 1], det[3].block_items[fin_pos - 1], det[3],
        #                            det[2]))

        #        del det[3].block_items[start_pos]
        #        del det[3].block_items[fin_pos - 1]

        cop = duplicate_element(ast)
        # TODO: Delete marker functions
        trees_dict, trees_paths_dict, is_job = get_paths_trees(cop, labs, labs, config.variables[i]['round'])

        print_rounds(labs, trees_dict, trees_paths_dict, config.variables[i]['round'], is_job,
                     config.delete_round_phase[i], config.msg_structure_fields[i],
                     config.variables[i], config.rounds_list[i])

        for el in nested_algos_details:
            if el[0] == i:

                # Nested algo should be in a Compound
                if isinstance(el[2], Compound):
                    p = find_parent(x, el[1].block_items[0])
                    ind = p.block_items.index(el[1].block_items[0])

                    while True:
                        elem = p.block_items[ind]
                        del p.block_items[ind]

                        if isinstance(elem, FuncCall) and elem.name.name == "marker_stop":
                            break
                    new_id = ID("inner_algorithm_" + letter, p.block_items[0].coord)
                    func = FuncCall(new_id, None, p.block_items[0].coord)
                    p.block_items.insert(ind, func)




        # Replace this algorithm with a function call
        #for tup in del_reminder:
        #    ind = tup[2].block_items.index(tup[0])
        #    while True:
        #        elem = tup[2].block_items[ind]
        #        del tup[2].block_items[ind]

        #        if elem == tup[1]:
        #            break
        #    new_id = ID("inner_algorithm_" + letter, tup[3].coord)
        #    func = FuncCall(new_id, None, tup[3].coord)
        #    tup[2].block_items.insert(ind, func)

        i -= 1

#conditii = []
#whiles_to_if(x, conditii)

#identify_recv_exits(x, conditii)
#remove_mbox(x, config.mailbox_1, config.clean_mailbox_1)

#take_code_from_file(ast, sys.argv[1], config.variables_1['round'],
#                    config.rounds_list_1, config.delete_round_phase, config.msg_structure_fields_1, config.variables_1)
