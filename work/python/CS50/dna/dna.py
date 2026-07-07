import csv
import sys


def main():

    if len(sys.argv) != 3:
        print("Usage: python dna.py data.csv sequence.txt")
        sys.exit(1)

    database = []
    with open(sys.argv[1]) as file:
        reader = csv.DictReader(file) 
        # DictReader()是取第一行作为key，再依次生成字典
        # 如{"name":"Alice", "AGATC":"2", ...}
        for row in reader:
            database.append(row)
    
    with open(sys.argv[2]) as file:
        sequence = file.read()

    strs = list(database[0].keys())[1:] #.keys()获取所有键的名字，然后跳过第一个name开始读，[1:]表示1到末尾
    profile = {}
    for s in strs:
        profile[s] = longest_match(sequence, s)

    for person in database:
        match = True
        for s in strs:
            if int(person[s]) != profile[s]:
                match = False
                break
        if match:
            print(person["name"])
            return

    print("No match")
    return


def longest_match(sequence, subsequence):
    """Returns length of longest run of subsequence in sequence."""

    # Initialize variables
    longest_run = 0
    subsequence_length = len(subsequence)
    sequence_length = len(sequence)

    # Check each character in sequence for most consecutive runs of subsequence
    for i in range(sequence_length):

        # Initialize count of consecutive runs
        count = 0

        # Check for a subsequence match in a "substring" (a subset of characters) within sequence
        # If a match, move substring to next potential match in sequence
        # Continue moving substring and checking for matches until out of consecutive matches
        while True:

            # Adjust substring start and end
            start = i + count * subsequence_length
            end = start + subsequence_length

            # If there is a match in the substring
            if sequence[start:end] == subsequence:
                count += 1
            
            # If there is no match in the substring
            else:
                break
        
        # Update most consecutive matches found
        longest_run = max(longest_run, count)

    # After checking for runs at each character in sequence, return longest run found
    return longest_run


main()
