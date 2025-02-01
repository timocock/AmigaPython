# Amiga-specific test cases
class AmigaPathTests(unittest.TestCase):
    def test_amiga_paths(self):
        self.assertEqual(posixpath.split("DH0:Python/Lib"), 
                       ("DH0:Python", "Lib")) 