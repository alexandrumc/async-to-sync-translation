from take_lab import take_code_from_file, get_extern_while_body_from_func, get_paths_trees, \
    turn_nested_algo_marked_compound, print_rounds, get_param_list
from pycparser import parse_file
from utils import duplicate_element, find_parent, get_global_vars, get_vars_table

from modify_whiles import *
import sys
import config
from mbox_removal import remove_mbox

ast = parse_file(filename=sys.argv[1], use_cpp=False)

global_vars = []

# For determining the global variables I will not use a
# visitor as I would have to jump over to many cases
get_global_vars(ast, global_vars)

# Construct a variables table with name : type
# used for determining the type of inferred for
# nested algos
vars_table = {}

x = get_extern_while_body_from_func(ast, "main")
get_vars_table(x, vars_table)
print vars_table


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

        cop = duplicate_element(ast)

        trees_dict, trees_paths_dict, is_job = get_paths_trees(cop, labs, labs, config.variables[i]['round'])

        print_rounds(labs, trees_dict, trees_paths_dict, config.variables[i]['round'], is_job,
                     config.delete_round_phase[i], config.msg_structure_fields[i],
                     config.variables[i], config.rounds_list[i])

        all_vars = get_param_list(trees_dict, i, global_vars, config.mailbox[i], vars_table)
        all_vars = list(map(lambda x: ID(x, None), all_vars))

        cop_all_vars = duplicate_element(all_vars)

        for el in nested_algos_details:
            if el[0] == i:
                p = find_parent(x, el[1].block_items[0])
                # Nested algo should be in a Compound
                if isinstance(p, Compound):
                    ind = p.block_items.index(el[1].block_items[0])

                    while True:
                        elem = p.block_items[ind]
                        del p.block_items[ind]

                        if isinstance(elem, FuncCall) and elem.name.name == "marker_stop":
                            break
                    new_id = ID("inner_algorithm_" + letter, p.block_items[0].coord)
                    func = FuncCall(new_id, ExprList(cop_all_vars, p.block_items[0].coord), p.block_items[0].coord)
                    p.block_items.insert(ind, func)

        i -= 1
