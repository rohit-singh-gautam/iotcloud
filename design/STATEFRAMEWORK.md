# State framework

This framework is designed to convert syncronous code to asyncronous code. This is done by maintaining state.

Statemachie is always single threaded though it is being made thread save by re-entrant function and a context.

Each context has its own state.

Context contains following inforamtion:
1. State
2. Temporary memory allocation API
3. Permanent memory allocation API

Permanent memory only used for creation of session. Each request in this session can be allocated from temporary memory.


Statemachine is predefined as integer. Integer is used for optimization.





