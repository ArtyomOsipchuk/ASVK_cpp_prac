import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

plt.style.use('default')
sns.set_palette("husl")

# Загрузка данных
df = pd.read_csv('results.csv')

# Расчет Pmut
df['Pmut'] = 0.0004 * (1.5 ** df['Series'])

# Группировка по Series для анализа стабильности
grouped = df.groupby('Series')

# Расчет разброса значений критерия (разница между худшим и лучшим в серии)
stability_data = []
for series, group in grouped:
    worst_best = group['Best'].max()  # Худший результат (максимальное значение критерия)
    best_best = group['Best'].min()   # Лучший результат (минимальное значение критерия)
    stability = worst_best - best_best  # Разброс (стабильность)
    stability_data.append({
        'Series': series,
        'Pmut': group['Pmut'].iloc[0],
        'Stability': stability,
        'Best_Best': best_best,
        'Worst_Best': worst_best
    })

stability_df = pd.DataFrame(stability_data)

# Создание графика
plt.figure(figsize=(12, 7))

# График стабильности
plt.plot(stability_df['Series'], stability_df['Stability'], 'o-', 
         linewidth=3, markersize=10, markerfacecolor='red', 
         markeredgecolor='darkred', markeredgewidth=2, label='Разброс критерия')

plt.xlabel('Series', fontsize=14, fontweight='bold')
plt.ylabel('Разброс критерия (стабильность)', fontsize=14, fontweight='bold')
plt.title('Зависимость стабильности алгоритма от Series\n(Разница между худшим и лучшим результатом в серии)', 
          fontsize=16, fontweight='bold', pad=20)

# Добавление значений на точки
for i, row in stability_df.iterrows():
    plt.annotate(f'{row["Stability"]:.3f}', 
                (row['Series'], row['Stability']), 
                textcoords="offset points", 
                xytext=(0,12), 
                ha='center',
                fontsize=11,
                fontweight='bold',
                bbox=dict(boxstyle="round,pad=0.3", facecolor="yellow", alpha=0.8))

# Добавление сетки
plt.grid(True, alpha=0.3, linestyle='--', which='both')

# Настройка осей
plt.xticks(stability_df['Series'], fontsize=12)
plt.yticks(fontsize=12)

# Добавление легенды
plt.legend(fontsize=12, loc='best')

plt.tight_layout()

plt.savefig('stability_vs_pmut.png', dpi=300, bbox_inches='tight')