import field_intersept

print("- declaring class")

class Foo(object):
    def __init__(self):
        print("- field updates in init")
        x = 1
        self.x = x
        self.y = 2
        self.z = self.y

print("- init starts")

f = Foo()

print("- init finished")

print("field updates outside")
f.x = 3
f.y = 4
f.z = 5

print("field update settatr")
print(f.x)
object.__setattr__(f, "x", 1)
print(f.x)


field_intersept.stop()