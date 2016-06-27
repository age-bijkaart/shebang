#!/bin/bash
cat >t3a <<\EOF
#!./shebang cat $<
line 1
line 2
EOF

cp t3a t3b
chmod +x t3a

cat >t3.expected <<\EOF
line 1
line 2
EOF

cat t3a >>t3.expected
cat t3b >>t3.expected

# resulting command: cat script t3a t3b >t3.out
./t3a t3[a-z] >t3.out && diff t3.expected t3.out && rm t3.out t3[a-z] t3.expected
