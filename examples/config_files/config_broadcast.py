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

    Having 3 algorithms, we will have a list with 3 structures for variables and a list with 3 structures for messages:

    variables.append(dict(
        phase = "phase_variable_for_A",
        round = "round_variable_for_A"
    ))

    variables.append(dict(
        phase = "phase_variable_for_B",
        round = "round_variable_for_B"
    ))

    variables.append(dict(
        phase = "phase_variable_for_C",
        round = "round_variable_for_C"
    ))

    The message structure for algorithm no. N has to specify the phase field and the round field for every algorithm
    that is before it in the call chain (N-1, N-2, ... 1) and we will also have a list with elements which represents
    the phase and round of every algorithm:

    msg_structure_fields.append(dict(
        phase_field_1 = "phase_field_for_A",
        round_field_1 = "round_field_for_A"
    ))

    msg_structure_fields.append(dict(
        phase_field_1 = "phase_field_for_A",
        round_field_1 = "round_field_for_A",

        phase_field_2 = "phase_field_for_B",
        round_field_2 = "round_field_for_B"
    ))

    msg_structure_fields.append(dict(
        phase_field_1 = "phase_field_for_A",
        round_field_1 = "round_field_for_A",

        phase_field_2 = "phase_field_for_B",
        round_field_2 = "round_field_for_B",

        phase_field_3 = "phase_field_for_C",
        round_field_3 = "round_field_for_C",
    ))

    Also, we have to specify the "mailbox" variable name for each algorithm. For our example with 3 algorithms:

    mailbox.append("mailbox_for_A")
    mailbox.append("mailbox_for_B")
    mailbox.append("mailbox_for_C")

    And the names of the functions that clean the mailboxes are also needed:

    clean_mailbox.append("function_for_A")
    clean_mailbox.append("function_for_B")
    clean_mailbox.append("function_for_C")

    For N algorithms, N declarations of each type (variables, msg_structure_fields, mailbox, clean_mailbox) are needed.
"""
######################################################################################################################


# Number of nested algorithms
number_of_nested_algorithms = 1

# We will use list of elements to represent details about each
# algorithm

variables = []

# Modify with phase and round variables names
variables.append(dict(
    phase="i",
    round="round"
))

msg_structure_fields = []

# Modify with phase and round fields names in the message structure
msg_structure_fields.append(dict(
    name="m",
    phase_field="i",
    round_field="round"
))

mailbox = []

# Modify with mailbox name
mailbox.append("mbox")

clean_mailbox = []

# Modify with clean mailbox function name
clean_mailbox.append("list_dispose")

rounds_list = []

rounds_list.append(['FIRST_ROUND', 'SECOND_ROUND', 'THIRD_ROUND', 'AUX_ROUND'])

delete_round_phase = []

delete_round_phase.append(True)

# Add as many fields as you need

######################################################################################################################
