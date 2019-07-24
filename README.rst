===============
ATHOS - Asynchronous To HO Synchronizer
===============

:Authors:
    Andrei Damian,
    Cezara Drăgoi,
    Alexandru Militaru,
    Josef Widder

.. contents::
    :backlinks: none

.. sectnum::


Introduction
============

What is ATHOS?
------------------

**ATHOS** is a tool that, given a communication-closed asynchronous protocol, soundly computes its round-based synchronous counterpart. 

Communication-closed protocol: the reception of a message is effectful only if its timestamp is equal to or greater than the local time of the receiver. In other words, stale messages are discarded.


What is it good for?
--------------------

Fault-tolerant distributed systems are implemented over asynchronous networks, so that they use algorithms for asynchronous models with faults. Due to asynchronous communication and the occurrence of faults (e.g., process crashes or the network dropping messages) the implementations are hard to understand and analyze. In contrast, synchronous computation models simplify design and reasoning. A synchronous version of your asynchronous protocol can be computed using **ATHOS**.

What class of algorithms can this tool translate?
------------------------------------------

**ATHOS** computes the synchronous counterpart of known consensus and leader election protocols, such as, Paxos, and Chandra and Toueg’s consensus.

Some other examples of protocols that we translated can be found here: `examples <https://github.com/alexandrumc/async-to-sync-translation/tree/master/examples/inputs-async/>`_.

Our output for the aforementioned list of examples can be found here: `outputs <https://github.com/alexandrumc/async-to-sync-translation/tree/master/examples/outputs-sync/>`_.

-----------------------------------

Using
==========
Prerequisites
-------------

* Python 2.7::

    sudo apt-get install python2.7

* Linux platform
* `Verifast Linux/x64 <http://82076e0e62875f063ae8-929808a701855dfb71539d0a4342d4be.r54.cf5.rackcdn.com/verifast-nightly.tar.gz>`_.


Steps
-----------------------------------

1. Clone project
2. Install Python
3. Download Verifast

Or you can download the virtual machine (with the setup already made) from here.


------------------------------------------

Basic usage
----------
First, you have to replace  the ``verifast_path`` variable from `here <https://github.com/alexandrumc/async-to-sync-translation/blob/0485622bd67d351e83d251aab56d8f006f18779f/run-translation#L22>`_ with the one on your file system.

Use the script located in the root of the project: ``run-translation``.
If you want to run the translation on all our examples (which are `here <https://github.com/alexandrumc/async-to-sync-translation/tree/master/examples/inputs-async/>`_) you should run the following command::

./run-translation -allExamples

If you want to run the translation without Verifast(i.e., without checking that the protocol is communication-closed, according to the criteria specified in `Section 5 <https://github.com/alexandrumc/async-to-sync-translation/blob/master/paper.pdf>`_.) you should disable it by adding the following flag::

./run-translation -allExamples -disableVerifast

Advanced usage
--------------

The tool can be also runned on a given input file. The command has the following format::

./run-translation [option] /path/to/file /path/to/config_file.py

If you want to run the translation on one of our examples (which are `here <https://github.com/alexandrumc/async-to-sync-translation/tree/master/examples/inputs-async/>`_) you will find the associated configuration file `here <https://github.com/alexandrumc/async-to-sync-translation/tree/master/examples/config_files>`_.

If you want to skip this verification phase, you should disable it by adding the flag::

./run-translation -disableVerifast /path/to/file /path/to/config_file.py

To run the tool on another protocol, you should write a configuration file for it first. An example of a configuration file and some instructions on how to write it can be found `here <https://github.com/alexandrumc/async-to-sync-translation/blob/master/ctc-translation/config.py>`_. 
Your protocol has to respect the following format:
    * Only C99-like syntactic constructions are allowed.
    * All the code has to be placed inside a ``main()`` function.
    * Directives and headers are not supported yet. As mentioned in the pycparser documentation, "the key point to understand here is that pycparser doesn't really care about the semantics of types. It only needs to know whether some token encountered in the source is a previously defined type. This is essential in order to be able to parse C correctly.".
    * ``if`` and ``else`` tokens are always followed by curly braces. 


Package contents
================

README.rst:
  This README file.

run-translation:
  Running script.

examples/inputs_async:
  A directory with some examples on which we runned **ATHOS**

examples/config_files:
  A directory with the configuration files for our examples.

examples/outputs_sync:
  A directory with the synchronous versions of our examples produced by **ATHOS**.
  
paper.pdf:
  Communication-closed asynchronous protocols paper.

ctc-translation/:
  The **ATHOS** source code.

pycparser/:
  The pycparser module source code.


Contributors
============

Andrei Damian - Politehnica University Bucharest

Cezara Drăgoi - INRIA, ENS, CNRS, PSL

Alexandru Militaru - Politehnica University Bucharest

Josef Widder - TU Wien

Special thanks go to the author of **pycparser**, Eli Bendersky. 


Contact
=========

For questions or any problems with the tool, you can `write us <cezara.dragoi@inria.fr>`_ to ask for help.
