import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt


df = pd.read_csv('results_seq.csv')
df_L = df[df['Cooling'] == 'L']
#heatmap_data = df.pivot_table(index='Processors', columns='Jobs', values='Time')

mean_data = df_L.groupby(['Jobs', 'Processors'])['Time'].mean().unstack()

# Cортировка оси Jobs по возрастанию (если нужно сверху вниз)
mean_data = mean_data.sort_index(ascending=False)

#heatmap_data = heatmap_data.sort_index(ascending=False)
plt.figure(figsize=(10, 7))
sns.heatmap(mean_data, annot=True, fmt='.0f', cmap='viridis')
plt.title('Time (Jobs vs Processors)')
plt.show()