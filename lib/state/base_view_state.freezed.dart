// coverage:ignore-file
// GENERATED CODE - DO NOT MODIFY BY HAND
// ignore_for_file: type=lint
// ignore_for_file: unused_element, deprecated_member_use, deprecated_member_use_from_same_package, use_function_type_syntax_for_parameters, unnecessary_const, avoid_init_to_null, invalid_override_different_default_values_named, prefer_expression_function_bodies, annotate_overrides, invalid_annotation_target

part of 'base_view_state.dart';

// **************************************************************************
// FreezedGenerator
// **************************************************************************

T _$identity<T>(T value) => value;

final _privateConstructorUsedError = UnsupportedError(
    'It seems like you constructed your class using `MyClass._()`. This constructor is only meant to be used by freezed and you are not supposed to need it nor use it.\nPlease check the documentation here for more information: https://github.com/rrousselGit/freezed#custom-getters-and-methods');

/// @nodoc
mixin _$BaseViewState {
  int get viewId => throw _privateConstructorUsedError;
  int get textureId => throw _privateConstructorUsedError;
  Size get surfaceSize => throw _privateConstructorUsedError;
  Rect get visibleBounds => throw _privateConstructorUsedError;
  List<int> get popups => throw _privateConstructorUsedError;
  Key get widgetKey => throw _privateConstructorUsedError;
  Key get textureKey => throw _privateConstructorUsedError;

  @JsonKey(ignore: true)
  $BaseViewStateCopyWith<BaseViewState> get copyWith =>
      throw _privateConstructorUsedError;
}

/// @nodoc
abstract class $BaseViewStateCopyWith<$Res> {
  factory $BaseViewStateCopyWith(
          BaseViewState value, $Res Function(BaseViewState) then) =
      _$BaseViewStateCopyWithImpl<$Res, BaseViewState>;
  @useResult
  $Res call(
      {int viewId,
      int textureId,
      Size surfaceSize,
      Rect visibleBounds,
      List<int> popups,
      Key widgetKey,
      Key textureKey});
}

/// @nodoc
class _$BaseViewStateCopyWithImpl<$Res, $Val extends BaseViewState>
    implements $BaseViewStateCopyWith<$Res> {
  _$BaseViewStateCopyWithImpl(this._value, this._then);

  // ignore: unused_field
  final $Val _value;
  // ignore: unused_field
  final $Res Function($Val) _then;

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? viewId = null,
    Object? textureId = null,
    Object? surfaceSize = null,
    Object? visibleBounds = null,
    Object? popups = null,
    Object? widgetKey = null,
    Object? textureKey = null,
  }) {
    return _then(_value.copyWith(
      viewId: null == viewId
          ? _value.viewId
          : viewId // ignore: cast_nullable_to_non_nullable
              as int,
      textureId: null == textureId
          ? _value.textureId
          : textureId // ignore: cast_nullable_to_non_nullable
              as int,
      surfaceSize: null == surfaceSize
          ? _value.surfaceSize
          : surfaceSize // ignore: cast_nullable_to_non_nullable
              as Size,
      visibleBounds: null == visibleBounds
          ? _value.visibleBounds
          : visibleBounds // ignore: cast_nullable_to_non_nullable
              as Rect,
      popups: null == popups
          ? _value.popups
          : popups // ignore: cast_nullable_to_non_nullable
              as List<int>,
      widgetKey: null == widgetKey
          ? _value.widgetKey
          : widgetKey // ignore: cast_nullable_to_non_nullable
              as Key,
      textureKey: null == textureKey
          ? _value.textureKey
          : textureKey // ignore: cast_nullable_to_non_nullable
              as Key,
    ) as $Val);
  }
}

/// @nodoc
abstract class _$$_BaseViewStateCopyWith<$Res>
    implements $BaseViewStateCopyWith<$Res> {
  factory _$$_BaseViewStateCopyWith(
          _$_BaseViewState value, $Res Function(_$_BaseViewState) then) =
      __$$_BaseViewStateCopyWithImpl<$Res>;
  @override
  @useResult
  $Res call(
      {int viewId,
      int textureId,
      Size surfaceSize,
      Rect visibleBounds,
      List<int> popups,
      Key widgetKey,
      Key textureKey});
}

/// @nodoc
class __$$_BaseViewStateCopyWithImpl<$Res>
    extends _$BaseViewStateCopyWithImpl<$Res, _$_BaseViewState>
    implements _$$_BaseViewStateCopyWith<$Res> {
  __$$_BaseViewStateCopyWithImpl(
      _$_BaseViewState _value, $Res Function(_$_BaseViewState) _then)
      : super(_value, _then);

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? viewId = null,
    Object? textureId = null,
    Object? surfaceSize = null,
    Object? visibleBounds = null,
    Object? popups = null,
    Object? widgetKey = null,
    Object? textureKey = null,
  }) {
    return _then(_$_BaseViewState(
      viewId: null == viewId
          ? _value.viewId
          : viewId // ignore: cast_nullable_to_non_nullable
              as int,
      textureId: null == textureId
          ? _value.textureId
          : textureId // ignore: cast_nullable_to_non_nullable
              as int,
      surfaceSize: null == surfaceSize
          ? _value.surfaceSize
          : surfaceSize // ignore: cast_nullable_to_non_nullable
              as Size,
      visibleBounds: null == visibleBounds
          ? _value.visibleBounds
          : visibleBounds // ignore: cast_nullable_to_non_nullable
              as Rect,
      popups: null == popups
          ? _value._popups
          : popups // ignore: cast_nullable_to_non_nullable
              as List<int>,
      widgetKey: null == widgetKey
          ? _value.widgetKey
          : widgetKey // ignore: cast_nullable_to_non_nullable
              as Key,
      textureKey: null == textureKey
          ? _value.textureKey
          : textureKey // ignore: cast_nullable_to_non_nullable
              as Key,
    ));
  }
}

/// @nodoc

class _$_BaseViewState with DiagnosticableTreeMixin implements _BaseViewState {
  const _$_BaseViewState(
      {required this.viewId,
      required this.textureId,
      required this.surfaceSize,
      required this.visibleBounds,
      required final List<int> popups,
      required this.widgetKey,
      required this.textureKey})
      : _popups = popups;

  @override
  final int viewId;
  @override
  final int textureId;
  @override
  final Size surfaceSize;
  @override
  final Rect visibleBounds;
  final List<int> _popups;
  @override
  List<int> get popups {
    // ignore: implicit_dynamic_type
    return EqualUnmodifiableListView(_popups);
  }

  @override
  final Key widgetKey;
  @override
  final Key textureKey;

  @override
  String toString({DiagnosticLevel minLevel = DiagnosticLevel.info}) {
    return 'BaseViewState(viewId: $viewId, textureId: $textureId, surfaceSize: $surfaceSize, visibleBounds: $visibleBounds, popups: $popups, widgetKey: $widgetKey, textureKey: $textureKey)';
  }

  @override
  void debugFillProperties(DiagnosticPropertiesBuilder properties) {
    super.debugFillProperties(properties);
    properties
      ..add(DiagnosticsProperty('type', 'BaseViewState'))
      ..add(DiagnosticsProperty('viewId', viewId))
      ..add(DiagnosticsProperty('textureId', textureId))
      ..add(DiagnosticsProperty('surfaceSize', surfaceSize))
      ..add(DiagnosticsProperty('visibleBounds', visibleBounds))
      ..add(DiagnosticsProperty('popups', popups))
      ..add(DiagnosticsProperty('widgetKey', widgetKey))
      ..add(DiagnosticsProperty('textureKey', textureKey));
  }

  @override
  bool operator ==(dynamic other) {
    return identical(this, other) ||
        (other.runtimeType == runtimeType &&
            other is _$_BaseViewState &&
            (identical(other.viewId, viewId) || other.viewId == viewId) &&
            (identical(other.textureId, textureId) ||
                other.textureId == textureId) &&
            (identical(other.surfaceSize, surfaceSize) ||
                other.surfaceSize == surfaceSize) &&
            (identical(other.visibleBounds, visibleBounds) ||
                other.visibleBounds == visibleBounds) &&
            const DeepCollectionEquality().equals(other._popups, _popups) &&
            (identical(other.widgetKey, widgetKey) ||
                other.widgetKey == widgetKey) &&
            (identical(other.textureKey, textureKey) ||
                other.textureKey == textureKey));
  }

  @override
  int get hashCode => Object.hash(
      runtimeType,
      viewId,
      textureId,
      surfaceSize,
      visibleBounds,
      const DeepCollectionEquality().hash(_popups),
      widgetKey,
      textureKey);

  @JsonKey(ignore: true)
  @override
  @pragma('vm:prefer-inline')
  _$$_BaseViewStateCopyWith<_$_BaseViewState> get copyWith =>
      __$$_BaseViewStateCopyWithImpl<_$_BaseViewState>(this, _$identity);
}

abstract class _BaseViewState implements BaseViewState {
  const factory _BaseViewState(
      {required final int viewId,
      required final int textureId,
      required final Size surfaceSize,
      required final Rect visibleBounds,
      required final List<int> popups,
      required final Key widgetKey,
      required final Key textureKey}) = _$_BaseViewState;

  @override
  int get viewId;
  @override
  int get textureId;
  @override
  Size get surfaceSize;
  @override
  Rect get visibleBounds;
  @override
  List<int> get popups;
  @override
  Key get widgetKey;
  @override
  Key get textureKey;
  @override
  @JsonKey(ignore: true)
  _$$_BaseViewStateCopyWith<_$_BaseViewState> get copyWith =>
      throw _privateConstructorUsedError;
}
