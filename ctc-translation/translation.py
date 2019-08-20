from pycparser import parse_file

from utils.utils import duplicate_element, find_parent, get_global_vars, get_vars_table, get_recv_whiles, \
                        get_main_function, get_extern_while_body

from main_logic.modify_whiles import *
from main_logic.mbox_removal import remove_mbox

from main_logic.take_lab import get_extern_while_body_from_func, get_paths_trees, \
    turn_nested_algo_marked_compound, print_rounds, get_param_list, turn_send_loops_funcs, syntax_each_algo, \
    add_old_vars_filter, apply_ifs_for_phasejumps, isolate_jump_phase

import config

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
get_vars_table(ast, vars_table)

# Identify recv loops
# Get main function from ast
main_func_node = None
try:
    main_func_node = get_main_function(ast)
except ValueError:
    print "Incorrect AST argument"

recv_loops = {}
recv_loops_out_internal = get_recv_whiles(main_func_node, config.rounds_list, config.msg_structure_fields, recv_loops)

# Get syntax for every algo
syntax_dict = {}
syntax_dict = syntax_each_algo(main_func_node, config.rounds_list, config.msg_structure_fields)

# Identify and already modify send loops, as recv loops
# may contain send loops - recovery case, for example
turn_send_loops_funcs(x, config.rounds_list, config.msg_structure_fields)

conditions = []
whiles_to_if(x, recv_loops, recv_loops_out_internal, syntax_dict, conditions)

identify_recv_exits(x, conditions)

# Uncomment this to print AST after while recv removal
"""
cgen = c_generator.CGenerator()
print cgen.visit(x)
raise AttributeError
"""

# Turn all while algos into marked compounds
nested_algos_details = []

turn_nested_algo_marked_compound(main_func_node, syntax_dict, nested_algos_details, config.rounds_list,
                                 config.msg_structure_fields)

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
if config.number_of_nested_algorithms > 1:
    print "Launched procedure for nested algorithms\n\n"
else:
    print "No inner algorithm detected\n"

# Iterate through algorithms and print them
i = len(config.rounds_list)
i -= 1

while i >= 0:
    letter = str(unichr(65 + i))

    if config.number_of_nested_algorithms > 1:
        print "Algorithm " + letter + "\n\n"

    labs = duplicate_element(config.rounds_list[i])
    labs.append('ERR_ROUND')

    cop = duplicate_element(ast)

    is_upon = False

    try:
        is_upon = syntax_dict[i]
    except KeyError:
        print "Found an unlabeled algorithm"

    trees_dict, trees_paths_dict, is_job = get_paths_trees(cop, labs, labs, config.variables[i]['round'], is_upon)

    if is_upon:
        is_job = True

        vars_list = []
        vars_list.append("pid")
        vars_list.append(config.msg_structure_fields[i]["name"])
        vars_list.append(config.msg_structure_fields[i]["round_field"])
        vars_list.append(config.mailbox[i])

        add_old_vars_filter(config.msg_structure_fields[i], vars_list, trees_dict, trees_paths_dict)
        for el in trees_paths_dict.keys():
            trees_paths_dict[el] = []

    # Check if this algo has phase jumps
    check_jumps = False
    for el in nested_algos_details:
        if el[0] == i and el[3]:
            check_jumps = True
            break

    if check_jumps:
        isolate_jump_phase(trees_dict, trees_paths_dict, i, config.msg_structure_fields)
        apply_ifs_for_phasejumps(trees_dict, trees_paths_dict)
        is_job = True

    print_rounds(labs, trees_dict, trees_paths_dict, config.variables[i]['round'], is_job,
                 config.delete_round_phase[i], config.msg_structure_fields[i], config.variables[i], is_upon)

    all_vars = get_param_list(trees_dict, i, global_vars, config.mailbox[i], vars_table,
                              config.mailbox, config.msg_structure_fields, config.rounds_list)
    all_vars = [ID(aux_node, None) for aux_node in all_vars]

    cop_all_vars = duplicate_element(all_vars)

    if i == 0:
        break
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
                new_id = ID("inner_algorithm_" + letter, p.coord)
                func = FuncCall(new_id, ExprList(cop_all_vars, p.coord), p.coord)
                p.block_items.insert(ind, func)

    i -= 1
