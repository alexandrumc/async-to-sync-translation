from take_lab import take_code_from_file, get_extern_while_body_from_func, get_context
from pycparser import parse_file

from modify_whiles import *
import config

generator = c_generator.CGenerator()

ast = parse_file(filename="../examples/c_files/verified_copy/Multi-Paxos-Roles.c", use_cpp=False)
extern_while_body = None
x = get_extern_while_body_from_func(ast, "main")
conditii = []
whiles_to_if(x, conditii)

identify_recv_exits(x, conditii)
remove_mbox(x, config.mailbox_1, config.clean_mailbox_1)
# remove_mbox(x, 'mboxB','list_dispose')


# rounds_list = ['FIRST_ROUND', 'SECOND_ROUND', 'THIRD_ROUND', 'FOURTH_ROUND']

take_code_from_file(ast, "../examples/c_files/verified_copy/Multi-Paxos-Roles.c", config.variables_1['round'],
                    config.rounds_list_1, config.delete_round_phase, config.msg_structure_fields_1, config.variables_1)
