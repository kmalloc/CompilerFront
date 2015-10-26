# **1. notes for regular expression**

## 1) syntax tree.
node type in a syntax tree for implementing NFA is described as follow:

| symbol                    | node type                                                           |
| ------------------        | ------------------------------------------------------------------- |
| `dot(.)`                  | Leaf Node                                                           |
| `^`                       | Head Node                                                           |
| `$`                       | Tail Node                                                           |
| `[]`                      | O-Node, option node.                                                |             |
| `*`                       | Star Node                                                           |
| `+`                       | Star Node                                                           |
| `?`                       | Star Node                                                           |
| `{}`                      | Star Node                                                           |
| `r1r2`                    | Cat Node                                                            |
| <code>r1&#124;r2</code>   | Or Node                                                             |
| `()`                      | represent a unit, it will be set as an attribute in the tree node.  |
| `\1`                      | leaf node, for back reference.                                      |
| `char`                    | leaf node represents normal character.                              |

## 2) usage.
syntaxs of the regular expression supported by my implementation.

1. all the syntaxs supported are as mentioned in the previous section.
2. for [], support range and negation, [a-f], [2-9], [^abc].
3. for back reference, referencing index starts from 0, this is quite different from what you usually see.
   eg, given pattern: ((ab)cd),  for testing string abcd, we get: \\0 == ab, \\1 == abcd.
   so, group capturing works inside out.
4. use {2, 4} to make repetition of the previous unit.
   eg, (ab){2, 4} means repeating ab 2 to 4 times(inclusive)

# **2. ink - a toy scripting language modeling the syntax of python and c(ongoing)**

