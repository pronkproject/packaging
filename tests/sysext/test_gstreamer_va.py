#!/usr/bin/python3
# SPDX-License-Identifier: MIT

import hashlib
import importlib.machinery
import importlib.util
import io
import pathlib
import tempfile
import unittest
from unittest import mock


ROOT = pathlib.Path(__file__).resolve().parents[2]
loader = importlib.machinery.SourceFileLoader(
    "gstreamer_va", str(ROOT / "scripts/build-gstreamer-va")
)
spec = importlib.util.spec_from_loader(loader.name, loader)
builder = importlib.util.module_from_spec(spec)
loader.exec_module(builder)


class RuntimeTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = pathlib.Path(self.temporary.name)
        self.output = self.root / "build"
        self.stage = self.root / "stage"
        plugin = self.output / "sys/va/libgstva.so"
        plugin.parent.mkdir(parents=True)
        plugin.write_bytes(b"plugin")
        for directory, name in [("codecs", "gstcodecs"),
                                ("codecparsers", "gstcodecparsers"), ("va", "gstva")]:
            parent = self.output / "gst-libs/gst" / directory
            parent.mkdir(parents=True)
            filename = f"lib{name}-1.0.so.0.2807.0"
            (parent / filename).write_bytes(b"library")
            (parent / f"lib{name}-1.0.so.0").symlink_to(filename)
            (parent / (filename + ".p")).mkdir()

    def test_only_runtime_files_are_staged(self):
        with mock.patch.object(builder, "run") as run:
            builder.stage_runtime(self.output, self.stage)
        self.assertEqual(run.call_count, 4)
        libraries = self.stage / "usr/lib64"
        self.assertEqual(len(list(libraries.rglob("*"))), 8)
        for name in ["gstcodecs", "gstcodecparsers", "gstva"]:
            link = libraries / f"lib{name}-1.0.so.0"
            self.assertTrue(link.is_symlink())
            self.assertEqual(link.read_bytes(), b"library")
        for call in run.call_args_list:
            self.assertEqual(call.args[:2], ("patchelf", "--remove-rpath"))

    def test_staging_replaces_old_plugin_symlink(self):
        sentinel = self.root / "outside"
        sentinel.write_bytes(b"untouched")
        plugin = self.stage / "usr/lib64/gstreamer-1.0/libgstva.so"
        plugin.parent.mkdir(parents=True)
        plugin.symlink_to(sentinel)
        with mock.patch.object(builder, "run"):
            builder.stage_runtime(self.output, self.stage)
            builder.stage_runtime(self.output, self.stage)
        self.assertFalse(plugin.is_symlink())
        self.assertEqual(plugin.read_bytes(), b"plugin")
        self.assertEqual(sentinel.read_bytes(), b"untouched")

    def test_missing_helper_fails(self):
        with mock.patch.object(pathlib.Path, "glob", return_value=[]):
            with self.assertRaisesRegex(SystemExit, "Missing VA runtime library"):
                builder.stage_runtime(self.output, self.stage)

    def test_wrong_gstreamer_version_fails_before_download(self):
        with mock.patch.object(builder.subprocess, "check_output", return_value="1.26.0\n"):
            with mock.patch.object(builder.urllib.request, "urlopen") as download:
                with self.assertRaisesRegex(SystemExit, "builder has 1.26.0"):
                    builder.build(self.root, self.stage, False)
                download.assert_not_called()

    def test_digest_matches_source_bytes(self):
        source = self.root / "source"
        source.write_bytes(b"pinned source")
        self.assertEqual(builder.digest(source), hashlib.sha256(b"pinned source").hexdigest())

    def test_bad_archive_is_rejected_before_patch_application(self):
        with mock.patch.object(builder.subprocess, "check_output", return_value="1.28.7\n"):
            with mock.patch.object(builder.urllib.request, "urlopen",
                                   return_value=io.BytesIO(b"wrong archive")):
                with mock.patch.object(builder, "run") as run:
                    with self.assertRaisesRegex(SystemExit, "source checksum mismatch"):
                        builder.build(self.root, self.stage, False)
                    archive = next((self.root / "builds/gstreamer-va").iterdir()) / \
                        "gst-plugins-bad-1.28.7.tar.xz"
                    self.assertFalse(archive.exists())
                    archive.write_bytes(b"corrupt cache")
                    with self.assertRaisesRegex(SystemExit, "Cached GStreamer"):
                        builder.build(self.root, self.stage, False)
                    run.assert_not_called()


if __name__ == "__main__":
    unittest.main()
