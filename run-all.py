import os

nodes_list = [2, 5, 10, 20, 30, 50]

os.system("make clean; make")

print("Start simulations")

for nodes in nodes_list:
    print("Simulation:", nodes)
    for sim_num in range(0,5):
        print("Run Test:", sim_num, "\n")
        os.system('cooja_nogui nd-test-mrm-' + str(nodes) + 'n.csc')
        print("Simulation ended storing...")
        os.system("python3 ./utils/arg-parser.py -i test.log -d dst_test/burst_20task_100off_350send_sim" + str(sim_num) + "/ -n " + str(nodes) + "  -c ./test_dc.log -r dst_test/burst_20task_100off_350send_sim" + str(sim_num) + "/raw")

