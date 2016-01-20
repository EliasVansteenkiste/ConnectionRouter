task=$1

if [[ "$2" == "-norun" ]]; then
    echo skip to populate
else
    ~/vtr/vtr_flow/scripts/run_vtr_task.pl $task
fi

~/benchtracker/populate_db.py $task --root_directory ~/vtr/vtr_flow/tasks/ \
--database ~/benchtracker/results.db -s \
"~/vtr/vtr_flow/scripts/parse_vtr_task.pl {task_dir} -run {run_number}" \
-k arch circuit
