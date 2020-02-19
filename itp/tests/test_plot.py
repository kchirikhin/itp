import itp.driver.plot as plt

import unittest


class TestYearsGenerator(unittest.TestCase):
    def test_next_returns_correct_three_values(self):
        generator = plt.YearsGenerator(2000)
        self.assertEqual(generator.next(), '2000')
        self.assertEqual(generator.next(), '2001')
        self.assertEqual(generator.next(), '2002')

    def test_generate_returns_correct_three_values(self):
        generator = plt.YearsGenerator(2000)
        _, tics = generator.generate(3)
        self.assertEqual(tics, ['2000', '2001', '2002'])

    def test_generate_returns_correct_three_positions(self):
        generator = plt.YearsGenerator(2000)
        positions, _ = generator.generate(3)
        self.assertEqual(positions, [0, 1, 2])

    def test_generate_throws_if_n_is_negative(self):
        generator = plt.YearsGenerator(2000)
        self.assertRaises(ValueError, generator.generate, -1)

    def test_generate_allows_to_skip_values(self):
        generator = plt.YearsGenerator(2000)
        positions, tics = generator.generate(9, skip=2)

        self.assertEqual(positions, [0, 3, 6])
        self.assertEqual(tics, ['2000', '2003', '2006'])


if __name__ == '__main__':
    unittest.main()
