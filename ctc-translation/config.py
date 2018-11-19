######################################################################################################################

"""
    Exemple of configuration file:

    Let A, B, C be 3 algorithms that follow the scheme:
        A -> B -> C, where "->" means "calls"
    i.e., A has a nested algorithm in it, B, and B in its turn has also a nested algorithm in it, C.

    For 3 nested algorithms:

    number_of_nested_algorithms = 3

    We need to specify the phase and round variables for each algorithm and how each message structure
    (for each algorithm) looks like.

    Having 3 algorithms, we will have 3 structures for variables and 3 structures for messages:

    variables_1 = dict(
        phase = "phase_variable_for_A",
        round = "round_variable_for_A"
    )

    variables_2 = dict(
        phase = "phase_variable_for_B",
        round = "round_variable_for_B"
    )

    variables_3 = dict(
        phase = "phase_variable_for_C",
        round = "round_variable_for_C"
    )

    The message structure for algorithm no. N has to specify the phase field and the round field for every algorithm
    that is before it in the call chain (N-1, N-2, ... 1) and also its own phase and round fields:

    msg_structure_fields_1 = dict(
        phase_field_1 = "phase_field_for_A",
        round_field_1 = "round_field_for_A"
    )

    msg_structure_fields_2 = dict(
        phase_field_1 = "phase_field_for_A",
        round_field_1 = "round_field_for_A",

        phase_field_2 = "phase_field_for_B",
        round_field_2 = "round_field_for_B"
    )

    msg_structure_fields_3 = dict(
        phase_field_1 = "phase_field_for_A",
        round_field_1 = "round_field_for_A",

        phase_field_2 = "phase_field_for_B",
        round_field_2 = "round_field_for_B",

        phase_field_3 = "phase_field_for_C",
        round_field_3 = "round_field_for_C",
    )

    Also, we have to specify the "mailbox" variable name for each algorithm. For our example with 3 algorithms:

    mailbox_1 = "mailbox_for_A"
    mailbox_2 = "mailbox_for_B"
    mailbox_3 = "mailbox_for_C"

    And the names of the functions that clean the mailboxes are also needed:

    clean_mailbox_1 = "function_for_A"
    clean_mailbox_2 = "function_for_B"
    clean_mailbox_3 = "function_for_C"

    For N algorithms, N declarations of each type (variables, msg_structure_fields, mailbox, clean_mailbox) are needed.
    Each name should end with "_algorithmNumber" so the algorithm can differentiate them.
"""
######################################################################################################################


# Number of nested algorithms
number_of_nested_algorithms = 2

# Modify with phase and round variables names
variables_1 = dict(
    phase="Insert PHASE variable name here",
    round="round",
    recv="recv"
)

variables_2 = dict(
    phase="Insert PHASE variable name here",
    round="bround",
    recv="recv"
)

# Modify with phase and round fields names in the message structure
msg_structure_fields_1 = dict(
    phase_field_1="Insert PHASE FIELD name here",
    round_field_1="Insert ROUND FIELD name here"
)

# Modify with mailbox name
mailbox_1 = "mbox"
mailbox_2 = "mboxB"

# Modify with clean mailbox function name
clean_mailbox_1 = "list_dispose"
clean_mailbox_2 = "list_dispose2"

rounds_list1 = ['NewEpoch', 'Ack_E',  'New_Leader', 'Ack_LD', 'BCAST']
rounds_list2 = ['FIRST_ROUND', 'SECOND_ROUND', 'THIRD_ROUND', 'AUX_ROUND']

# Add as many fields as you need

######################################################################################################################
