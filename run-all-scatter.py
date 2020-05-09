import os

nodes_list = [2, 5, 10, 20, 30, 50]
experiment = 15


os.system("make clean; make")

print("Start simulations")


print("\n\n\nExperiment number:", experiment)

for nodes in nodes_list:
    print("\n\nSimulation:", nodes)
    for sim_num in range(0,5):
        print("Run Test:", sim_num, "\n")
        os.system('cooja_nogui nd-test-mrm-' + str(nodes) + 'n.csc')
        print("Simulation ended storing...")
        os.system("python3 ./utils/arg-parser.py -i test.log -d dst_test/scatter_" + str(experiment) + "task_sim" + str(sim_num) + "/ -n " + str(nodes)
                  + "  -c ./test_dc.log -r dst_test/scatter_" + str(experiment) + "task_sim" + str(sim_num) + "/raw")
