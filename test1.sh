#!/bin/bash
cat >t1 <<\EOF
#!./shebang cat $<
line 1
line 2
EOF

chmod +x t1

cat >t1.expected <<\EOF
line 1
line 2
EOF

./t1 >t1.out && diff t1.expected t1.out && rm t1.out t1 t1.expected
