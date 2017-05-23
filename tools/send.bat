rem del /s/q bin\*.*
rem javac -d bin -cp src;lib/jssc.jar -source 1.6 -target 1.6 src/Test.java
java -cp bin;lib/jssc.jar Test %*

