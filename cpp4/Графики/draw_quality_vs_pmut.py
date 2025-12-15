import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

plt.style.use('default')
sns.set_palette("husl")

# Загрузка данных
df = pd.read_csv('results.csv')

# Добавляем Pmut
df['Pmut'] = 0.0004 * (1.5 ** df['Series'])

# Находим лучшее качество для каждой серии
best_by_series = df.groupby('Series')['Best'].min().reset_index()
best_by_series['Pmut'] = 0.0004 * (1.5 ** best_by_series['Series'])

# Создаем график
plt.figure(figsize=(12, 8))

# Вариант 1: Используем Series для оси X (как запрошено)
plt.subplot(2, 2, 1)
plt.plot(best_by_series['Series'], best_by_series['Best'], 'o-', 
         linewidth=2, markersize=10, 
         markerfacecolor='red', markeredgecolor='darkred', markeredgewidth=1,
         label='Лучшее качество')

plt.xlabel('Series', fontsize=12)
plt.ylabel('Лучшее значение критерия', fontsize=12)
plt.title('Зависимость качества от Series', fontsize=14, fontweight='bold')
plt.grid(True, alpha=0.3, linestyle='--')

# Добавляем аннотации
for i, (series, best) in enumerate(zip(best_by_series['Series'], best_by_series['Best'])):
    plt.annotate(f'{best:.3f}', 
                (series, best), 
                textcoords="offset points", 
                xytext=(0,10), 
                ha='center',
                fontsize=9,
                bbox=dict(boxstyle="round,pad=0.3", facecolor="yellow", alpha=0.7))

plt.legend()

plt.tight_layout()
plt.savefig('quality_vs_pmut.png', dpi=300, bbox_inches='tight')