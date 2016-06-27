#!/bin/bash
cat >t2 <<\EOF
#!./shebang cat $<
line 1
line 2
EOF

chmod +x t2

cat >t2.expected <<\EOF
line 1
line 2
EOF

cat t2 >>t2.expected
cat t2 >>t2.expected

./t2 t2 t2 >t2.out && diff t2.expected t2.out && rm t2.out t2 t2.expected
