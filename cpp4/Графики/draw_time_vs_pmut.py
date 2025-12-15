import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

plt.style.use('default')
sns.set_palette("husl")

# Загрузка данных
df = pd.read_csv('results.csv')

# Расчет Pmut (поскольку в файле нет Nproc, используем Series)
df['Pmut'] = 0.0004 * (1.5 ** df['Series'])

# Группировка данных по Series для усреднения времени
df_grouped = df.groupby('Series').agg({
    'Time': ['mean', 'std'],  # среднее и стандартное отклонение
    'Pmut': 'first'           # Pmut одинаков для всех строк с одинаковым Series
}).reset_index()

# Упрощаем названия столбцов
df_grouped.columns = ['Series', 'Time_mean', 'Time_std', 'Pmut']

# Сортировка по Series (на всякий случай)
df_grouped = df_grouped.sort_values('Series')

# Создание графика
plt.figure(figsize=(10, 6))

# График с точками и линиями
plt.errorbar(df_grouped['Series'], df_grouped['Time_mean'], 
             yerr=df_grouped['Time_std'],
             fmt='o-', linewidth=2, markersize=8, 
             markerfacecolor='red', markeredgecolor='darkred', 
             markeredgewidth=1, capsize=5, capthick=2,
             ecolor='gray', alpha=0.7)

plt.xlabel('Series (номер серии)', fontsize=12)
plt.ylabel('Время выполнения (секунды)', fontsize=12)
plt.title('Зависимость времени выполнения от номера серии', 
          fontsize=14, fontweight='bold')

plt.grid(True, alpha=0.3, linestyle='--')

# Добавление аннотаций для каждой точки
for i, row in df_grouped.iterrows():
    plt.annotate(f'{row["Time_mean"]:.1f}s', 
                (row['Series'], row['Time_mean']), 
                textcoords="offset points", 
                xytext=(0,10), 
                ha='center',
                fontsize=9,
                bbox=dict(boxstyle="round,pad=0.3", facecolor="yellow", alpha=0.7))
    
    # Также можно добавить значение Pmut в подсказку
    plt.annotate(f'Pmut={row["Pmut"]:.6f}', 
                (row['Series'], row['Time_mean']), 
                textcoords="offset points", 
                xytext=(0,-20), 
                ha='center',
                fontsize=8,
                color='blue',
                bbox=dict(boxstyle="round,pad=0.2", facecolor="lightblue", alpha=0.5))

# Устанавливаем целочисленные значения на оси X (так как Series - целые числа)
plt.xticks(df_grouped['Series'])

plt.tight_layout()

# Сохранение графика
plt.savefig('time_vs_series.png', dpi=300, bbox_inches='tight')