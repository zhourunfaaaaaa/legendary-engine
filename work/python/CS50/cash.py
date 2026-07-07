from cs50 import get_float
while True:
    dollars = get_float("Change owed: ")
    if dollars >= 0:
        break

cents = round(dollars)

coins = 0
for coin in [25, 10, 5, 1]:
    coins += cents // coin # //在python里是整除
    cents %= coin

print(coins)