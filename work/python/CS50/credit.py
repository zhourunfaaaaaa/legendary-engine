from cs50 import get_string
from cs50 import get_int

number = get_string("Number: ")

total = 0
for i, ch in enumerate(reversed(number)):
    digit = int(ch)
    if i % 2 == 1:
        digit *= 2
        if digit >= 10:
            digit -= 9
    total += digit

if total % 10 != 0:
    print("INVALID")
else:
    length = len(number)
    if length == 15 and number[:2] in ("34", "37"): #[:2]是从0到2
        print("AMEX")
    elif length == 16 and "51" <= number[:2] <= "55":
        print("MASTERCARD")
    elif length in (13, 16) and number[0] == "4":
        print("VISA")
    else:
        print("INVALID")