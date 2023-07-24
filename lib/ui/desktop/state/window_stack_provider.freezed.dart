// coverage:ignore-file
// GENERATED CODE - DO NOT MODIFY BY HAND
// ignore_for_file: type=lint
// ignore_for_file: unused_element, deprecated_member_use, deprecated_member_use_from_same_package, use_function_type_syntax_for_parameters, unnecessary_const, avoid_init_to_null, invalid_override_different_default_values_named, prefer_expression_function_bodies, annotate_overrides, invalid_annotation_target, unnecessary_question_mark

part of 'window_stack_provider.dart';

// **************************************************************************
// FreezedGenerator
// **************************************************************************

T _$identity<T>(T value) => value;

final _privateConstructorUsedError = UnsupportedError(
    'It seems like you constructed your class using `MyClass._()`. This constructor is only meant to be used by freezed and you are not supposed to need it nor use it.\nPlease check the documentation here for more information: https://github.com/rrousselGit/freezed#custom-getters-and-methods');

/// @nodoc
mixin _$WindowStackState {
  List<int> get stack => throw _privateConstructorUsedError;

  @JsonKey(ignore: true)
  $WindowStackStateCopyWith<WindowStackState> get copyWith =>
      throw _privateConstructorUsedError;
}

/// @nodoc
abstract class $WindowStackStateCopyWith<$Res> {
  factory $WindowStackStateCopyWith(
          WindowStackState value, $Res Function(WindowStackState) then) =
      _$WindowStackStateCopyWithImpl<$Res, WindowStackState>;
  @useResult
  $Res call({List<int> stack});
}

/// @nodoc
class _$WindowStackStateCopyWithImpl<$Res, $Val extends WindowStackState>
    implements $WindowStackStateCopyWith<$Res> {
  _$WindowStackStateCopyWithImpl(this._value, this._then);

  // ignore: unused_field
  final $Val _value;
  // ignore: unused_field
  final $Res Function($Val) _then;

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? stack = null,
  }) {
    return _then(_value.copyWith(
      stack: null == stack
          ? _value.stack
          : stack // ignore: cast_nullable_to_non_nullable
              as List<int>,
    ) as $Val);
  }
}

/// @nodoc
abstract class _$$_WindowStackStateCopyWith<$Res>
    implements $WindowStackStateCopyWith<$Res> {
  factory _$$_WindowStackStateCopyWith(
          _$_WindowStackState value, $Res Function(_$_WindowStackState) then) =
      __$$_WindowStackStateCopyWithImpl<$Res>;
  @override
  @useResult
  $Res call({List<int> stack});
}

/// @nodoc
class __$$_WindowStackStateCopyWithImpl<$Res>
    extends _$WindowStackStateCopyWithImpl<$Res, _$_WindowStackState>
    implements _$$_WindowStackStateCopyWith<$Res> {
  __$$_WindowStackStateCopyWithImpl(
      _$_WindowStackState _value, $Res Function(_$_WindowStackState) _then)
      : super(_value, _then);

  @pragma('vm:prefer-inline')
  @override
  $Res call({
    Object? stack = null,
  }) {
    return _then(_$_WindowStackState(
      stack: null == stack
          ? _value._stack
          : stack // ignore: cast_nullable_to_non_nullable
              as List<int>,
    ));
  }
}

/// @nodoc

class _$_WindowStackState extends _WindowStackState {
  const _$_WindowStackState({required final List<int> stack})
      : _stack = stack,
        super._();

  final List<int> _stack;
  @override
  List<int> get stack {
    if (_stack is EqualUnmodifiableListView) return _stack;
    // ignore: implicit_dynamic_type
    return EqualUnmodifiableListView(_stack);
  }

  @override
  String toString() {
    return 'WindowStackState(stack: $stack)';
  }

  @override
  bool operator ==(dynamic other) {
    return identical(this, other) ||
        (other.runtimeType == runtimeType &&
            other is _$_WindowStackState &&
            const DeepCollectionEquality().equals(other._stack, _stack));
  }

  @override
  int get hashCode =>
      Object.hash(runtimeType, const DeepCollectionEquality().hash(_stack));

  @JsonKey(ignore: true)
  @override
  @pragma('vm:prefer-inline')
  _$$_WindowStackStateCopyWith<_$_WindowStackState> get copyWith =>
      __$$_WindowStackStateCopyWithImpl<_$_WindowStackState>(this, _$identity);
}

abstract class _WindowStackState extends WindowStackState {
  const factory _WindowStackState({required final List<int> stack}) =
      _$_WindowStackState;
  const _WindowStackState._() : super._();

  @override
  List<int> get stack;
  @override
  @JsonKey(ignore: true)
  _$$_WindowStackStateCopyWith<_$_WindowStackState> get copyWith =>
      throw _privateConstructorUsedError;
}
