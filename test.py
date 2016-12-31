# -*- coding: utf-8 -*-
import unittest
import subprocess


class cccTest(unittest.TestCase):

    def test_hoge(self):
        expected = '0'
        cmd = "ls"
        subprocess.call(cmd, shell=True)

        #  actual = "neko"
        #  self.assertEqual(expected, actual)


if __name__ == '__main__':
    unittest.main()
