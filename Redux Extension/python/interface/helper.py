import ast
import random
import string


def getCompiledOutput(data):
    return ast.literal_eval(data.replace('false', '"false"').replace('true', '"true"'))


def stringGenerator(size=6, chars=string.ascii_uppercase):
    return ''.join(random.choice(chars) for _ in range(size))
