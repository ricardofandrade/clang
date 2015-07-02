//===--- DeclFriend.cpp - C++ Friend Declaration AST Node Implementation --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the AST classes related to C++ friend
// declarations.
//
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclFriend.h"
#include "clang/AST/DeclTemplate.h"
using namespace clang;

void FriendDecl::anchor() { }

FriendDecl *FriendDecl::getNextFriendSlowCase() {
  return cast_or_null<FriendDecl>(
                           NextFriend.get(getASTContext().getExternalSource()));
}

FriendDecl *FriendDecl::Create(ASTContext &C, DeclContext *DC,
                               SourceLocation L,
                               FriendUnion Friend,
                               SourceLocation FriendL,
                               SourceLocation UsingL,
                        ArrayRef<TemplateParameterList*> FriendTypeTPLists) {
#ifndef NDEBUG
  if (Friend.is<NamedDecl*>()) {
    NamedDecl *D = Friend.get<NamedDecl*>();
    assert(isa<FunctionDecl>(D) ||
           isa<CXXRecordDecl>(D) ||
           isa<FunctionTemplateDecl>(D) ||
           isa<ClassTemplateDecl>(D));

    // As a temporary hack, we permit template instantiation to point
    // to the original declaration when instantiating members.
    assert(D->getFriendObjectKind() ||
           (cast<CXXRecordDecl>(DC)->getTemplateSpecializationKind()));
    // These template parameters are for friend types only.
    assert(FriendTypeTPLists.size() == 0);
  }
#endif

  std::size_t Size = sizeof(FriendDecl)
    + FriendTypeTPLists.size() * sizeof(TemplateParameterList*);
  void *Mem = C.Allocate(Size);
  FriendDecl *FD = new (Mem) FriendDecl(DC, L, Friend, FriendL, UsingL,
                                        FriendTypeTPLists);
  cast<CXXRecordDecl>(DC)->pushFriendDecl(FD);
  return FD;
}

FriendDecl *FriendDecl::CreateDeserialized(ASTContext &C, unsigned ID,
                                           unsigned FriendTypeNumTPLists) {
  std::size_t Size = sizeof(FriendDecl)
    + FriendTypeNumTPLists * sizeof(TemplateParameterList*);
  void *Mem = AllocateDeserializedDecl(C, ID, Size);
  return new (Mem) FriendDecl(EmptyShell(), FriendTypeNumTPLists);
}

FriendDecl *CXXRecordDecl::getFirstFriend() const {
  ExternalASTSource *Source = getParentASTContext().getExternalSource();
  Decl *First = data().FirstFriend.get(Source);
  return First ? cast<FriendDecl>(First) : 0;
}
