generate:

    find -L samples | grep \\.root$ | sort | tee remote_run.txt

submit:

    hep_sub remote_run.sh -argu %{ProcId} -n $(wc -l remote_run.txt | egrep -o '[0-9]+') -o log/%{ProcId}_1.log -e log/%{ProcId}_2.log
