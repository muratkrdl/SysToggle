import json
import sys
from pathlib import Path

REFERENCE = "en-US"
UNUSED_KEYS = (("settings", "language_en"), ("settings", "language_tr"))


def flatten(node, prefix=""):
    if isinstance(node, dict):
        for key, value in node.items():
            yield from flatten(value, f"{prefix}{key}/")
    else:
        yield prefix.rstrip("/"), node


def drop_unused(path):
    data = json.loads(path.read_text(encoding="utf-8"))
    changed = False
    for section, key in UNUSED_KEYS:
        if key in data.get(section, {}):
            del data[section][key]
            changed = True
    if changed:
        path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")


def check(root, fix):
    i18n = Path(root) / "resources" / "i18n"
    if fix:
        for app in i18n.glob("*/app.json"):
            drop_unused(app)

    problems = 0
    for name in ("app.json", "hints.json"):
        reference = dict(flatten(json.loads((i18n / REFERENCE / name).read_text(encoding="utf-8"))))
        for locale_dir in sorted(p for p in i18n.iterdir() if p.is_dir()):
            strings = dict(flatten(json.loads((locale_dir / name).read_text(encoding="utf-8"))))
            missing = reference.keys() - strings.keys()
            extra = strings.keys() - reference.keys()
            wrong = [k for k in reference.keys() & strings.keys()
                     if str(reference[k]).count("{}") != str(strings[k]).count("{}")]
            for label, keys in (("missing", missing), ("extra", extra), ("placeholder mismatch", wrong)):
                for key in sorted(keys):
                    problems += 1
                    print(f"{locale_dir.name}/{name}: {label}: {key}")
        print(f"{name}: checked {len(list(i18n.iterdir()))} locales")
    return problems


if __name__ == "__main__":
    count = check(sys.argv[1], "--fix" in sys.argv)
    print(f"{count} problem(s)")
    sys.exit(1 if count else 0)
