from take_lab import take_code_from_file, get_extern_while_body_from_func, get_paths_trees, turn_nested_algo_if_stm, print_rounds
from pycparser import parse_file
from utils import duplicate_element

from modify_whiles import *
import sys
import config

ast = parse_file(filename=sys.argv[1], use_cpp=False)
x = get_extern_while_body_from_func(ast, "main")

# If we have only one algorithm then process it directly
if config.number_of_nested_algorithms == 1:
    conditions = []
    whiles_to_if(x, conditions)

    identify_recv_exits(x, conditions)
    remove_mbox(x, config.mailbox_1, config.clean_mailbox_1)

    take_code_from_file(ast, sys.argv[1], config.variables[0]['round'],
                        config.rounds_list_1, config.delete_round_phase, config.msg_structure_fields[0],
                        config.variables[0])
else:
    # Turn all while algos into if (cond) loop_body
    conditions = []

    turn_nested_algo_if_stm(x, conditions)

    conditions = []
    whiles_to_if(x, conditions)

    identify_recv_exits(x, conditions)

    # Delete unnecessary operations, like disposes and timeouts from code
    # First, get a list of all messages names

    mess_names = []
    for d in config.msg_structure_fields:
        mess_names.append(d['name'])

    remove_mbox(x, mess_names)

    cop = duplicate_element(ast)

    # Test reasons
    labs = duplicate_element(config.rounds_list[1])
    labs.append('ERR_ROUND')

    trees_dict, trees_paths_dict, is_job = get_paths_trees(cop, labs, labs, config.variables[1]['round'])

    rounds_list = [config.rounds_list[1]]

    # print generator.visit(cop)
    print_rounds(labs, trees_dict, trees_paths_dict, config.variables[1]['round'], is_job, config.delete_round_phase,
                 config.msg_structure_fields[1], config.variables[1],
                 rounds_list[0])



#conditii = []
#whiles_to_if(x, conditii)

#identify_recv_exits(x, conditii)
#remove_mbox(x, config.mailbox_1, config.clean_mailbox_1)

#take_code_from_file(ast, sys.argv[1], config.variables_1['round'],
#                    config.rounds_list_1, config.delete_round_phase, config.msg_structure_fields_1, config.variables_1)
