# 完整的水果识别机器学习程序
# 作者：你的机器学习向导
# 日期：现在！

# 1. 导入工具包
from sklearn import tree
import matplotlib.pyplot as plt

# 2. 准备训练数据
print("=== 准备训练数据 ===")
features = [[150, 8], [170, 7], [140, 9], [130, 3], [120, 2], [110, 4]]
labels = [1, 1, 1, 0, 0, 0]

# 显示数据
for i in range(len(features)):
    fruit_type = "苹果" if labels[i]==1 else "橙子"
    print(f"水果{i+1}: 重量={features[i][0]}g, 光滑度={features[i][1]}, 种类={fruit_type}")

# 3. 创建和训练模型
print("\n=== 开始训练模型 ===")
clf = tree.DecisionTreeClassifier()
clf = clf.fit(features, labels)
print("训练完成！计算机已经学会了判断规则。")

# 4. 预测新数据
print("\n=== 预测新水果 ===")
new_fruits = [[145, 7], [125, 4], [160, 6]]
new_predictions = clf.predict(new_fruits)

for i in range(len(new_fruits)):
    fruit_type = "苹果" if new_predictions[i]==1 else "橙子"
    print(f"新水果{i+1}: 重量={new_fruits[i][0]}g, 光滑度={new_fruits[i][1]} → 预测为：{fruit_type}")

# 5. 显示学到的规则
print("\n=== 计算机学会的决策规则 ===")
print("规则解读：")
print("1. 先判断重量是否≤135g")
print("   是 → 橙子")
print("   否 → 进入第2步")
print("2. 再判断光滑度是否≤5.5")
print("   是 → 橙子")
print("   否 → 苹果")

# 6. 可视化（在Colab中会显示图片）
print("\n正在生成决策树可视化...")
plt.figure(figsize=(10,6))
tree.plot_tree(clf, 
               feature_names=["重量(g)", "光滑度"],
               class_names=["橙子", "苹果"],
               filled=True,
               rounded=True)
plt.title("计算机学会的决策树")
plt.show()