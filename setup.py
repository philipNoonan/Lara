#!/usr/bin/env python

"""The setup script."""

import os
from setuptools import setup, find_packages

requirements = [
    'glfw',
    'PyOpenGL',
    'opencv-python',
    'imgui',
    'flask',
    'pyaudio'
]

setup(
    author='PN',
    author_email='philip.noonan@outlook.com',
    python_requires='>=3.6',
    entry_points={
        'console_scripts': [
            'lara=lara.lara:main',
        ],
    },
    install_requires=requirements,
    include_package_data=True,
    name='lara',
    packages=find_packages(include=['lara', 'lara.*']),
    setup_requires=[],
    test_suite='tests',
    tests_require=[],
    url='https://github.com/philipNoonan/lara',
    version='0.1.0',
    zip_safe=False,
)
