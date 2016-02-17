#!/usr/bin/env python3

import random, tempfile, os

TEST_DIR = "tests"
N_TESTS = 100
INIT_SIZE = 100

# Positive tests:
# 1. Push n elements, pop m <= n elements.
#       Check that elements pop in the right order.
# 2. Repeat the above k times.

# Negative tests:
# 1. Pop from an empty queue.
# 2. Push n elements, pop n + m elements.

def genPushPop(size, queue):
  n = random.randint(0, size)
  m = random.randint(0, n)
  pushes = []
  for i in range(n):
    pri = random.randint(0, size)
    queue.append(pri)
    pushes.append(pri)

  queue.sort(reverse=True)

  outputs = queue[:m]

  return (pushes, m, outputs, queue[m:])

def genPushPops(size):
  k = random.randint(1, size)
  queue = []
  retval = []
  for i in range(k):
    t = genPushPop(size // k, queue)
    (_, _, _, queue) = t
    retval.append(t)
  return retval

def writePushPop(t, xFile, yFile):
  (xs, m, ys, _) = t

  for x in xs:
    xFile.write(str(x) + "\n")

  xFile.write("p\n" * m)

  for y in ys:
    yFile.write("=> " + str(y) + "\n")

def writePushPops(ts, inFile, expFile):
  for t in ts:
    writePushPop(t, inFile, expFile)

def runTest():
  (_, inPath) = tempfile.mkstemp(prefix="bouncer.", dir=TEST_DIR)
  expPath = inPath + ".exp"
  outPath = inPath + ".out"

  inFile = open(inPath, "w")
  expFile = open(expPath, "w")

  ts = genPushPops(INIT_SIZE)
  writePushPops(ts, inFile, expFile)

  inFile.close()
  expFile.close()

  os.system("cat \"" + inPath + "\" | ./bouncer &> \"" + outPath + "\"")

  if os.system("diff \"" + expPath + "\" \"" + outPath + "\"") != 0:
    print("Failed: \"" + inPath + "\"")
    print("Input: \n" + open(inPath).read() + "\n")
    print("Expected: \n" + open(expPath).read() + "\n")
    print("Got: \n" + open(outPath).read() + "\n")
    retval = False
  else:
    os.remove(outPath)
    os.remove(expPath)
    os.remove(inPath)
    retval = True

  return retval

def runTests():
  n_success = 0
  for i in range(N_TESTS):
    if runTest():
      n_success += 1

  print("Ran %d test(s), %d failed." % (N_TESTS, N_TESTS - n_success))

def main():

  if not os.path.exists(TEST_DIR):
    os.mkdir(TEST_DIR)

  runTests()

  try:
    os.rmdir(TEST_DIR)
  except:
    pass

if __name__ == "__main__":
  main()
