#!/bin/bash

nTarefas=$1
ficheiro=$2
ficheiroScript="${ficheiro}.speedups.csv"
ficheiroOut="${ficheiro}.res"
contadorTarefas=1

while [ $contadorTarefas -le $nTarefas ]
do
	if [ $contadorTarefas -eq 1 ]
	then
		./CircuitRouter-SeqSolver $ficheiro
		timeSeq=$(grep "Elapsed time" $ficheiroOut | cut -f 4)
		touch $ficheiroScript
		echo "1S,${timeSeq},1" >> $ficheiroScript
	fi

	./CircuitRouter-ParSolver -t $contadorTarefas $ficheiro
	timePar=$(grep "Elapsed time" $ficheiroOut | cut -f 4)
	speedup=$(echo "scale=6; ${timeSeq}/${timePar}" | bc)
	echo "${contadorTarefas},${timePar},${speedup}" >> $ficheiroScript
	((contadorTarefas++))
done
