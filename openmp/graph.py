import matplotlib.pyplot as plt

# Número de threads/processos
num_configurations = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]  #threads/processos

# Tempos de execução para cada configuração
static_times = [1699.08 , 891.408 , 629.151 , 489.667, 401.746, 432.355, 359.784,
                 359.808, 320.381, 285.631, 261.614, 242.312, 224.496, 210.354, 208.604, 202.056]  # Tempos static

dynamic_times = [1687.88 , 874.943 , 654.217, 475.244, 388.504, 367.231, 305.981, 282.406, 265.395, 
                 242.585, 249.002, 219.862, 210.569, 200.405, 190.503, 182.152]  # Tempos dynamic

# Calcular o speedup em relação à execução static
speedup = [static_times[0] / dynamic_times for dynamic_times in dynamic_times]

# Plotar o gráfico
plt.figure(figsize=(8, 6))
plt.plot(num_configurations, speedup, marker='o', linestyle='-')
plt.title('Gráfico de Speedup')
plt.xlabel('Número de Threads/Processos')
plt.ylabel('Speedup')
plt.grid(True)
plt.show()