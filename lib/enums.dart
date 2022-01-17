enum XdgSurfaceRole {
  none,
  toplevel,
  popup,
}

enum Edges {
  none,
  top,
  bottom,
  left,
  right,
}

extension EdgesExt on Edges {
  int get id {
    switch (this) {
      case Edges.none:
        return 0;
      case Edges.top:
        return 1 << 0;
      case Edges.bottom:
        return 1 << 1;
      case Edges.left:
        return 1 << 2;
      case Edges.right:
        return 1 << 3;
    }
  }

  bool operator &(int bitmap) {
    return bitmap & id != 0;
  }
}