import random
import string
import unittest
from cython_wrappers import py_byteshift_strstr, py_byteshift_memmem


class TestStrStrBase(object):
    def search(self, haystack, needle):
        raise NotImplementedError

    def get_rand_char(self):
        raise NotImplementedError

    def do_test(self, haystack, needle, *args, **kw):
        if isinstance(haystack, str):
            haystack = haystack.encode('ascii')
        if isinstance(needle, str):
            needle = needle.encode('ascii')
        return self.assertEqual(
            self.search(haystack, needle),
            haystack.find(needle),
            *args, **kw
        )

    def test_both_empty(self):
        self.do_test('', '')

    def test_empty_haystack(self):
        self.do_test('', 'x')

    def test_empty_needle(self):
        self.do_test('x', '')

    def test_identical(self):
        self.assertEqual(self.search(b'abc', b'abc'), 0)

    def test_found_at_middle(self):
        haystack = b'XXXneedleXXX'
        needle = b'needle'
        self.assertEqual(self.search(haystack, needle), 3)

    def test_found_at_start(self):
        haystack = b'needleXXX'
        needle = b'needle'
        self.assertEqual(self.search(haystack, needle), 0)

    def test_found_at_end(self):
        haystack = b'XXXneedle'
        needle = b'needle'
        self.assertEqual(self.search(haystack, needle), 3)

    def test_found_multiple_times(self):
        haystack = b'XXXneedleXXXneedleXXXneedleXXX'
        needle = b'needle'
        self.assertEqual(self.search(haystack, needle), 3)

    def test_needle_lengths(self):
        for needle_len in range(1, 66):
            needle = b'x' * needle_len
            self.assertEqual(self.search(b'', needle), -1)
            self.assertEqual(self.search(needle, needle), 0)
            self.assertEqual(self.search(b'yyy' + needle, needle), 3)
            self.assertEqual(self.search(needle + b'zzz', needle), 0)
            self.assertEqual(self.search(b'yyy' + needle + b'zzz', needle), 3)

    def test_big_needle_in_bigger_haystack(self):
        haystack = ''.join([
            'VFBMuzumaBIKeTBQOjZqlbuUkxPrAtEBnjRFBtRdfQMggzwIbSDmXCDMpgZMntbqq',
            'PxCwcgLgcarMgIOZSCFYQHFSQlCgcmMGLlHpGtbmlIcvadsYHMFJrMjzFEbEDZdCt',
            'DcvfhzIXlxcCSijvFiuvMlPKHeGzMtjojedhWRRUQXxmMIyrRoShKfpTlzPOiUlwy',
            'EuXoIiSbzwGWOnwsZybBbsoPKjolBYrECzBKwZsYKbLjXfYFzfqwGjDXNDtVacVIC',
            'jDsezgZUSMtVOeRTtFOPwFrieRYcabHBSPKRoFpJnicKdYFTMgrGnkXxxMsrhqQlf',
            'DmJEkuIKvbQKaIeIHvCFFhwDqEFfKFmlktCVhOECxbdcGaJqtCYnZwqGfDpeKdnNJ',
            'fjfcvbHhiEHwecofVyZPuvFpDQgrCtHvGOGMqeFZNlnPirtIvOSJvQLwLouTpzQZB',
            'UevcRndgiUTcHulJapqMEogYltkmspWVFQwUiUYXvYDvtBZZhYSpWESpsEVVlpBJo',
            'UGtbJwxSwrNuoOsGTrbdFVewciVCCBFZdIdMVQsbteJWgzPfPZViYFzVGQvSiXhSs',
            'KliFtsiWgcETMQQErxSZwUCSpfeXNmNAFjBVmJLzfFxPgixBUdBoGlkCZRNFFmSHP',
            'sr',
            ])
        needle = 'ICjDsezgZUSMtVOeRTtFOPwFrieRY'
        self.do_test(haystack, needle)

    def test_random_identical(self):
        import random
        for n_test in range(10):
            length = random.randint(0, 1000)
            needle = bytes([
                self.get_rand_char()
                for _i in range(length)
            ])
            self.do_test(needle, needle)

    def test_random_letters(self):
        for n_test in range(10):
            haystack = ''.join([
                random.choice(string.ascii_letters)
                for _i in range(random.randint(0, 1000))
            ])
            needle = ''.join([
                random.choice(string.ascii_letters)
                for _i in range(random.randint(0, 200))
            ])
            self.do_test(haystack, needle)

            idx = random.randint(0, len(haystack))
            haystack_with_needle = haystack[:idx] + needle + haystack[idx:]
            self.do_test(haystack_with_needle, needle, haystack_with_needle + ' ||| ' + needle)

    def test_random(self):
        for n_test in range(10):
            haystack = bytes([
                self.get_rand_char()
                for _i in range(random.randint(0, 1000))
            ])
            needle = bytes([
                self.get_rand_char()
                for _i in range(random.randint(0, 200))
            ])
            self.do_test(haystack, needle)

            idx = random.randint(0, len(haystack))
            haystack_with_needle = haystack[:idx] + needle + haystack[idx:]
            self.do_test(haystack_with_needle, needle)


class TestByteshiftStrStr(TestStrStrBase, unittest.TestCase):
    def search(self, haystack, needle):
        return py_byteshift_strstr(haystack, needle)

    def get_rand_char(self):
        return random.randint(1, 255)

    def test_not_past_null_byte(self):
        haystack = b'XXX\0needleXXX'
        needle = b'needle'
        self.assertEqual(self.search(haystack, needle), -1)


class TestByteshiftMemMem(TestStrStrBase, unittest.TestCase):
    def search(self, haystack, needle):
        return py_byteshift_memmem(haystack, needle)
    
    def get_rand_char(self):
        return random.randint(0, 255)

    def test_past_null_byte(self):
        haystack = b'XXX\0needleXXX'
        needle = b'needle'
        self.assertEqual(self.search(haystack, needle), 4)
