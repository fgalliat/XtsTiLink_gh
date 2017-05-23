rem del /s/q bin\*.*
rem javac -d bin -cp src -source 1.6 -target 1.6 src/BinToH.java
java -cp bin BinToH %* > %1.h

