"""Verify split pack integrity and independently installable release archives."""
import argparse
import hashlib
import json
from pathlib import Path
import zipfile

parser = argparse.ArgumentParser()
parser.add_argument('world', type=Path)
parser.add_argument('text', type=Path)
parser.add_argument('--archives', action='store_true')
args = parser.parse_args()
roots = {'world': args.world, 'text-interface': args.text}
data = json.loads((args.world / 'pack-split.json').read_text())
assert data == json.loads((args.text / 'pack-split.json').read_text())
names = {}
ids = set()
for kind, root in roots.items():
    expected = {r['path']: r['sha256'] for r in data['textures'] if r['pack'] == kind}
    actual = {p.relative_to(root).as_posix(): p for p in (root / 'textures').rglob('*') if p.is_file()}
    assert actual.keys() == expected.keys(), f'{kind}: missing or unexpected textures'
    for path, p in actual.items():
        assert hashlib.sha256(p.read_bytes()).hexdigest() == expected[path], path
        names.setdefault(p.name, set()).add(kind)
    metadata = json.loads((root / 'mod.json').read_text())
    assert metadata['id'] not in ids
    ids.add(metadata['id'])
    assert 'port' not in metadata.get('compatibility', {})
    if kind == 'world':
        assert not (root / 'src').exists() and not (root / 'assets').exists()
        assert 'abi' not in metadata.get('compatibility', {})
    else:
        assert metadata['compatibility']['abi'] == 2
        assert (root / 'assets/glyphs.bin').read_bytes()[:4] == b'GTHD'
    print(f'{kind}: {len(actual)} textures verified')
    if args.archives:
        archives = list((root / 'build').rglob('*.fox')) + list((root / 'build').rglob('*.zip'))
        assert archives, f'{kind}: no release archives'
        for archive in archives:
            with zipfile.ZipFile(archive) as z:
                entries = z.namelist()
                assert len(entries) == len(set(entries)), 'Duplicate archive entries'
                prefix = '' if archive.suffix == '.fox' else ('sfa-hd-textures/' if kind == 'world' else 'sfa-hd-text/')
                assert json.loads(z.read(prefix + 'mod.json')) == metadata
                textures = {n[len(prefix):] for n in entries if n.startswith(prefix+'textures/') and not n.endswith('/')}
                assert textures == expected.keys(), archive
                for path in expected:
                    assert hashlib.sha256(z.read(prefix+path)).hexdigest() == expected[path], path
                assert prefix+'README.md' in entries and prefix+'LICENSE' in entries
                libraries = [n for n in entries if n.startswith(prefix+'lib/') and not n.endswith('/')]
                assert bool(libraries) == (kind == 'text-interface')
                if kind == 'text-interface':
                    assert prefix+'assets/glyphs.bin' in entries
            print(f'Archive verified: {archive.name}')
assert all(len(kinds) == 1 for kinds in names.values()), 'Texture replacement filename shared between packs'
print('PASS: texture checksums, split coverage, mod identities and compatibility declarations')
