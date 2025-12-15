import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

plt.style.use('default')
sns.set_palette("husl")

df = pd.read_csv('results.csv')

plt.figure(figsize=(10, 6))

plt.plot(df['Nproc'], df['Time'], 'o-', linewidth=2, markersize=8, 
         markerfacecolor='red', markeredgecolor='darkred', markeredgewidth=1)

plt.xlabel('Количество потоков (Nproc)', fontsize=12)
plt.ylabel('Время выполнения (секунды)', fontsize=12)
plt.title('Зависимость времени выполнения от количества протоков', fontsize=14, fontweight='bold')

plt.grid(True, alpha=0.3, linestyle='--')

for i, (nproc, time) in enumerate(zip(df['Nproc'], df['Time'])):
    plt.annotate(f'{time:.1f}s', 
                (nproc, time), 
                textcoords="offset points", 
                xytext=(0,10), 
                ha='center',
                fontsize=9,
                bbox=dict(boxstyle="round,pad=0.3", facecolor="yellow", alpha=0.7))

plt.xscale('log')
plt.xticks(df['Nproc'], df['Nproc'])

plt.tight_layout()

plt.savefig('graph_2.png', dpi=300, bbox_inches='tight')