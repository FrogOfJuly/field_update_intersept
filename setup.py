from setuptools import Extension, setup

setup(
    ext_modules=[
        Extension(
            name="field_intersept",
            sources=["src/fint.c"],
        ),
    ]
)
